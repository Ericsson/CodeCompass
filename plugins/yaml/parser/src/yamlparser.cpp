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
size_t file_get_contents(const char *filename, CharContainer *v)
{
    ::FILE *fp = ::fopen(filename, "rb");
    C4_CHECK_MSG(fp != nullptr, "could not open file");
    ::fseek(fp, 0, SEEK_END);
    long sz = ::ftell(fp);
    v->resize(static_cast<typename CharContainer::size_type>(sz));
    if(sz)
    {
        ::rewind(fp);
        size_t ret = ::fread(&(*v)[0], 1, v->size(), fp);
        C4_CHECK(ret == (size_t)sz);
    }
    ::fclose(fp);
    return v->size();
}

/** load a file from disk into an existing CharContainer */
template<class CharContainer>
CharContainer file_get_contents(const char *filename)
{
    CharContainer cc;
    file_get_contents(filename, &cc);
    return cc;
}

/** save a buffer into a file */
template<class CharContainer>
void file_put_contents(const char *filename, CharContainer const& v, const char* access)
{
    file_put_contents(filename, v.empty() ? "" : &v[0], v.size(), access);
}

/** save a buffer into a file */
void file_put_contents(const char *filename, const char *buf, size_t sz, const char* access)
{
    ::FILE *fp = ::fopen(filename, access);
    C4_CHECK_MSG(fp != nullptr, "could not open file");
    ::fwrite(buf, 1, sz, fp);
    ::fclose(fp);
}



YamlParser::YamlParser(ParserContext& ctx_): AbstractParser(ctx_)
{

  ///QUESTION: Should I add YamlContent db object to the _fileIdCache as well? 

  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::Yaml& yf  ///QUESTION: Tried model::Yaml here as well
      : _ctx.db->query<model::Yaml>())
    {
      _fileIdCache.insert(yf.file);
    }
  });

  int threadNum = 1;//_ctx.options["jobs"].as<int>();///Not needed probably
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

bool YamlParser::cleanupDatabase()///I guess it comes later, can be omitted
{
  if (!_fileIdCache.empty())
  {
    try
    {
      util::OdbTransaction {_ctx.db} ([this] {
        for (const model::File& file
          : _ctx.db->query<model::File>(
          odb::query<model::File>::id.in_range(_fileIdCache.begin(), _fileIdCache.end())))
        {
          auto it = _ctx.fileStatus.find(file.path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[yamlparser] Database cleanup: " << file.path;

            _ctx.db->erase_query<model::Yaml>(odb::query<model::Yaml>::file == file.id);
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
      // auto cb = accept(path);
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
///So, as per my understanding, this getParserCallback should return true
///Only for yaml files and the dirs which contain yaml files
util::DirIterCallback YamlParser::getParserCallback()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);

    if (boost::filesystem::is_regular_file(path))
    {
      // if (accept(currPath_))
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

std::string split (const std::string &s, bool isSeq) {
    std::vector<std::string> result;

    std::stringstream ss (s);
    std::string tmp;
    getline(ss, tmp, '\n');
    std::string item;

    while (getline (ss, item, '\n')) {
        if (isSeq) {
            item = std::regex_replace(item, std::regex("^\\s+-"), std::string(""));
        }
        result.push_back (item);
    }
    std::string list = "[";
    for (auto s : result)
        list += s + ",";
    list.pop_back();
    list += "]";
    return list;
}
void YamlParser::getstr(ryml::NodeRef node, ryml::csubstr parent, std::vector<keyData> &vec)
{
    
    auto getkey = [](ryml::NodeRef node){ return node.has_key() ? node.key() : ryml::csubstr{}; };
    auto getval = [](ryml::NodeRef node){ return node.has_val() ? node.val() : ryml::csubstr{}; };
    if(!node.is_container())
        vec.push_back(keyData(getkey(node), parent, getval(node)));
    else {
        std::string src = ryml::emitrs<std::string>(node);
        if (node.is_seq())
        {
          std::string data = split(src, true); 
          vec.push_back(keyData(getkey(node), parent, ryml::to_csubstr(data) ));
          for (ryml::NodeRef n : node.children())
          {
            if (n.is_container())
              getstr(n, getkey(node), vec);
          }
        }
        else if (node.is_map())
        {
          std::string data = split(src, false); 
          vec.push_back(keyData(getkey(node), parent, ryml::to_csubstr(data)));
          for (ryml::NodeRef n : node.children())
          {
              getstr(n, getkey(node), vec);
          }
        }
    }
}

/**
* Getting Loc(useful data) and putting it into the db, it is actually populating the
* db object we created in model/yaml.h. It is using transaction
*/
// void YamlParser::persistData(const std::vector<keyData>& data_, model::FileId file_, Type type)
void YamlParser::persistData(model::FilePtr file_)//, model::FileId fileId_)
{
  model::Yaml::Type type;
  std::vector<keyData> keysDataPairs;
  std::vector<char> content = file_get_contents<std::vector<char>>(file_->path.c_str());
  ryml::Tree yamlTree = ryml::parse_in_place(ryml::to_substr(content));
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
  else if (isCI(file_->path, "ci.yaml") || isCI(file_->path, "ci.yml") )
  {
    type = model::Yaml::CI;
  }
  else 
  {
    type = model::Yaml::OTHER;
  }
  ryml::NodeRef root = yamlTree.rootref();
  getstr(root, "", keysDataPairs);
  
  // for (ryml::NodeRef n : root.children())
  // {
  //   keysDataPairs.push_back(keyData(n.has_key() ? n.key() : ryml::csubstr{}, n.has_val() ? n.val() : ryml::csubstr{}));
  // }
  int i = 0;
  util::OdbTransaction trans(_ctx.db);
  trans([&, this]{
    for (keyData kd : keysDataPairs)
    {
      i++;
      if (i==1) continue;
      LOG(info) << "In PersistData: YamlKeydata is: " << kd;
   
      model::YamlContent yamlContent;
      yamlContent.file = file_->id;
      // if (yamlTree[kd.key].is_keyval())
      {
        std::stringstream ss;
        ss << kd.key;
        yamlContent.key = ss.str();///ryml::emitrs<std::string>(kd.key);
        ss.str("");
        ss << kd.data;
        yamlContent.data = ss.str();
        yamlContent.parent = kd.parent;
      }
      // else if (root[kd.key].is_seq())
      // {
      //   LOG(info) << "It is a sequence"<<std::endl;
      //   /*for (auto it = yamlTree[kd.key].begin(); it != yamlTree[kd.key].end(); ++it)
      //   {
      //     LOG(warning) << *it;
      //   }*/
      //   std::function<bool(const ryml::NodeRef*, size_t)> print;
      //   print = [&, this](const ryml::NodeRef* node, size_t ind) -> bool
      //   {
      //     if (node->has_children())
      //       for (const auto c : node->children())
      //       {
      //         LOG(warning) << c;
      //         print(&c, ind);
      //       }
      //     else
      //     {
      //       //LOG(warning) << node->val();
      //     }
      //      return true;
      //   };
      //   for (auto c : yamlTree[kd.key].children())
      //   {
      //     c.visit(print, 3);
      //   }
      //   std::stringstream ss;
      //   ss << kd.key;
      //   yamlContent.key = ss.str();///ryml::emitrs<std::string>(kd.key);
      //   ss.str("");
      //   for (const auto& v : yamlTree[kd.key])
      //   {
      //     ss << v << " ";
      //   }

      //   yamlContent.data = ss.str();
      // }
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
