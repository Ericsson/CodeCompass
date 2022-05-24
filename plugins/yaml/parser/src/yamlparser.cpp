#include <iterator>
#include <fstream>
#include <memory>
#include <functional>
#include <regex>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <parser/sourcemanager.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <model/yaml.h>
#include <model/yaml-odb.hxx>

#include <model/yamlcontent.h>
#include <model/yamlcontent-odb.hxx>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "yamlparser/ryml_all.hpp"

#include <yamlparser/yamlparser.h>

namespace cc
{
namespace parser
{

template<class CharContainer>
size_t fileGetContents(const char* filename_, CharContainer* v_)
{
  ::FILE* fp = ::fopen(filename_, "rb");
  C4_CHECK_MSG(fp != nullptr, "could not open file");
  ::fseek(fp, 0, SEEK_END);
  long sz = ::ftell(fp);
  v_->resize(static_cast<typename CharContainer::size_type>(sz));
  if(sz)
  {
    ::rewind(fp);
    size_t ret = ::fread(&(*v_)[0], 1, v_->size(), fp);
    C4_CHECK(ret == (size_t)sz);
  }
  ::fclose(fp);
  return v_->size();
}

/** load a file from disk into an existing CharContainer */
template<class CharContainer>
CharContainer fileGetContents(const char* filename_)
{
  CharContainer cc;
  fileGetContents(filename_, &cc);
  return cc;
}

YamlParser::YamlParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::Yaml& yf
      : _ctx.db->query<model::Yaml>())
    {
      _fileIdCache.insert(yf.file);
    }
  });

  int threadNum = _ctx.options["jobs"].as<int>();
  _pool = util::make_thread_pool<std::string>(
    threadNum, [this](const std::string& path_)
    {
      model::FilePtr file = _ctx.srcMgr.getFile(path_);
      if (file)
      {
        if (_fileIdCache.find(file->id) == _fileIdCache.end())
        {
          if (accept(file->path)) 
          {
            this->persistData(file);
            ++this->_visitedFileCount;
            file->parseStatus = model::File::PSFullyParsed;
            _ctx.srcMgr.updateFile(*file);
          }
        }
        else
          LOG(debug) << "YamlParser already parsed this file: " << file->path;
      }
    });
}

bool YamlParser::cleanupDatabase()
{
  if (!_fileIdCache.empty())
  {
    try
    {
      util::OdbTransaction {_ctx.db} ([this] {
        for (const model::File& file
          : _ctx.db->query<model::File>(
          odb::query<model::File>::id.in_range(
          _fileIdCache.begin(), _fileIdCache.end())))
        {
          auto it = _ctx.fileStatus.find(file.path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[yamlparser] Database cleanup: " << file.path;

            _ctx.db->erase_query<model::Yaml>(
              odb::query<model::Yaml>::file == file.id);
            _fileIdCache.erase(file.id);
          }
        }
      });
    }
    catch (odb::database_exception&)
    {
      LOG(fatal) << "Transaction failed in yaml parser!";
      return false;
    }
  }
  return true;
}

bool YamlParser::parse()
{
  this->_visitedFileCount = 0;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Yaml parse path: " << path;

    util::OdbTransaction trans(_ctx.db);
    trans([&, this]() {
      auto cb = getParserCallback();
      /*--- Call non-empty iter-callback for all files
         in the current root directory. ---*/
      try
      {
        util::iterateDirectoryRecursive(path, cb);
      }
      catch (std::exception& ex_)
      {
        LOG(warning)
          << "Yaml parser threw an exception: " << ex_.what();
      }
      catch (...)
      {
        LOG(warning)
          << "Yaml parser failed with unknown exception!";
      }
    });
  }

  _pool->wait();
  LOG(info) << "Processed files: " << this->_visitedFileCount;

  return true;
}

util::DirIterCallback YamlParser::getParserCallback()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);

    if (boost::filesystem::is_regular_file(path))
    {
      _pool->enqueue(currPath_);
    }
    
    return true;
  };
}

bool YamlParser::accept(const std::string& path_) const
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".yaml" || ext == ".yml";
}

std::string YamlParser::getDataFromNode(
  const std::string &node_,
  const bool isSeq_)
{
  std::vector<std::string> children;
  std::stringstream ss(node_);
  std::string nodeKey;
  getline(ss, nodeKey, '\n');
  std::string childItem;

  while (getline (ss, childItem, '\n')) {
    if (isSeq_) {
      childItem = std::regex_replace(childItem, std::regex(
        "^\\s+-"), std::string(""));
    }
    children.push_back (childItem);
  }
  if (children.empty())
    return "";

  std::string childrenList = "[";
  for (auto child : children)
      childrenList += child + ",";
  childrenList.pop_back();
  childrenList += "]";

  return childrenList;
}

void YamlParser::getKeyDataFromTree(
  ryml::NodeRef node_,
  ryml::csubstr parent_,
  std::vector<keyData>& dataVec_)
{
  auto getKey = [](ryml::NodeRef node_)
  { 
    return node_.has_key() ? node_.key() : ryml::csubstr{}; 
  };
  auto getVal = [](ryml::NodeRef node_)
  { 
    return node_.has_val() ? node_.val() : ryml::csubstr{}; 
  };
  if(!node_.is_container())
    dataVec_.push_back(keyData(getKey(node_), parent_, getVal(node_)));
  else {
    std::string nodeData = ryml::emitrs<std::string>(node_);
    if (node_.is_seq())
    {
      std::string childData = getDataFromNode(nodeData, true); 

      dataVec_.push_back(keyData(
        getKey(node_), parent_, ryml::to_csubstr(childData)));

      for (ryml::NodeRef nodeChild : node_.children())
      {
        if (nodeChild.is_container())
          getKeyDataFromTree(nodeChild, getKey(node_), dataVec_);
      }
    }
    else if (node_.is_map())
    {
      std::string childData = getDataFromNode(nodeData, false);

      if (getKey(node_) != "")
      {
        dataVec_.push_back(keyData(
          getKey(node_), parent_, ryml::to_csubstr(childData)));
      }

      for (ryml::NodeRef nodeChild : node_.children())
      {
        getKeyDataFromTree(nodeChild, getKey(
          node_) != "" ? getKey(node_) : parent_ , dataVec_);
      }
    }
  }
}

void YamlParser::persistData(model::FilePtr file_)
{
  model::Yaml::Type type;
  std::vector<keyData> keyDataPairs;
  std::vector<char> fileContents
    = fileGetContents<std::vector<char>>(file_->path.c_str());

  ryml::Tree yamlTree = ryml::parse_in_place(ryml::to_substr(fileContents));
  if (yamlTree["apiVersion"].has_key()) 
  {
    if (yamlTree["name"].has_key() && yamlTree["version"].has_key())
    {
      type = model::Yaml::HELM_CHART;
    }
    else if (yamlTree["kind"].has_key())
    {
      type = model::Yaml::KUBERNETES_CONFIG;
    }
  }
  else if (isCIFile(file_->path, "ci.yaml") || isCIFile(file_->path, "ci.yml"))
  {
    type = model::Yaml::CI;
  }
  else if (file_->filename == "docker-compose.yml")
  {
    type = model::Yaml::DOCKER_COMPOSE;
  }
  else
  {
    type = model::Yaml::OTHER;
  }

  ryml::NodeRef root = yamlTree.rootref();
  getKeyDataFromTree(root, "", keyDataPairs);

  util::OdbTransaction trans(_ctx.db);
  trans([&, this]{
    for (keyData kd : keyDataPairs)
    {
      model::YamlContent yamlContent;
      yamlContent.file = file_->id;
      yamlContent.key = kd.key;
      yamlContent.data = kd.data;
      yamlContent.parent = kd.parent;
      _ctx.db->persist(yamlContent);
    }

    model::Yaml yaml;
    yaml.file = file_->id;
    yaml.type = type;
    _ctx.db->persist(yaml);
  });
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Yaml Plugin");
    return description;
  }

  std::shared_ptr<YamlParser> make(ParserContext& ctx_)
  {
    return std::make_shared<YamlParser>(ctx_);
  }
}
#pragma clang diagnostic pop

}
}
