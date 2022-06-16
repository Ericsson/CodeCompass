#include <iterator>
#include <fstream>
#include <memory>
#include <functional>
#include <regex>

#include "yaml-cpp/yaml.h"

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>
#include <util/util.h>

#include <parser/sourcemanager.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <model/yaml.h>
#include <model/yaml-odb.hxx>
#include <model/yamlcontent.h>
#include <model/yamlcontent-odb.hxx>
#include <model/yamlastnode.h>
#include <model/yamlastnode-odb.hxx>

#include "yamlparser/yamlparser.h"

namespace cc
{
namespace parser
{

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
          //this->persistData(file);
          collectAstNodes(file);
          ++this->_visitedFileCount;
          file->parseStatus = model::File::PSFullyParsed;
          file->type = "YAML";
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
  //util::persistAll(_astNodes, _ctx.db);
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

void YamlParser::collectAstNodes(model::FilePtr file_)
{
  YAML::Node currentFile = YAML::LoadFile(file_->path);
  for (auto it = currentFile.begin(); it != currentFile.end(); ++it)
  {
    switch (it->first.Type())
    {
      case YAML::NodeType::Null:
        LOG(info) << it->first << ": null";
        break;
      case YAML::NodeType::Scalar:
        LOG(info) << it->first << ": Scalar";
        processScalar(it->first, file_, model::YamlAstNode::SymbolType::Key);
        break;
      case YAML::NodeType::Sequence:
        LOG(info) << it->first << ": Sequence";
        processSequence(it->first, file_, model::YamlAstNode::SymbolType::Key);
        break;
      case YAML::NodeType::Map:
        LOG(info) << it->first << ": Map";
        processMap(it->first, file_, model::YamlAstNode::SymbolType::Key);
        break;
      case YAML::NodeType::Undefined:
        LOG(info) << it->first << ": Undefined";
        break;
    }

    switch (it->second.Type())
    {
      case YAML::NodeType::Null:
        LOG(info) << it->second << ": null";
        break;
      case YAML::NodeType::Scalar:
        LOG(info) << it->second << ": Scalar";
        processScalar(it->second, file_, model::YamlAstNode::SymbolType::Value);
        break;
      case YAML::NodeType::Sequence:
        LOG(info) << it->second << ": Sequence";
        processSequence(it->second, file_, model::YamlAstNode::SymbolType::Value);
        break;
      case YAML::NodeType::Map:
        LOG(info) << it->second << ": Map";
        processMap(it->second, file_, model::YamlAstNode::SymbolType::Value);
        break;
      case YAML::NodeType::Undefined:
        LOG(info) << it->second << ": Undefined";
        break;
    }
  }
}

void YamlParser::processScalar(
  YAML::Node& node_,
  model::FilePtr file_,
  model::YamlAstNode::SymbolType symbolType_)
{
  model::YamlAstNodePtr currentNode = std::make_shared<model::YamlAstNode>();
  currentNode->astValue = node_.Scalar();
  currentNode->location.file = file_;
  currentNode->location.range = getNodeLocation(node_);
  currentNode->astType = model::YamlAstNode::AstType::SCALAR;
  currentNode->symbolType = symbolType_;
  currentNode->entityHash = util::fnvHash(YAML::Dump(node_));
  currentNode->id = model::createIdentifier(*currentNode);
  _astNodes.push_back(currentNode);
}

void YamlParser::processMap(
  YAML::Node& node_,
  model::FilePtr file_,
  model::YamlAstNode::SymbolType symbolType_)
{
  model::YamlAstNodePtr currentNode = std::make_shared<model::YamlAstNode>();
  currentNode->astValue = YAML::Dump(node_);
  currentNode->location.file = file_;
  currentNode->location.range = getNodeLocation(node_);
  currentNode->astType = model::YamlAstNode::AstType::MAP;
  currentNode->symbolType = symbolType_;
  currentNode->entityHash = util::fnvHash(YAML::Dump(node_));
  currentNode->id = model::createIdentifier(*currentNode);
  _astNodes.push_back(currentNode);
}

void YamlParser::processSequence(
  YAML::Node& node_,
  model::FilePtr file_,
  model::YamlAstNode::SymbolType symbolType_)
{
  model::YamlAstNodePtr currentNode = std::make_shared<model::YamlAstNode>();
  currentNode->astValue = YAML::Dump(node_);
  currentNode->location.file = file_;
  currentNode->location.range = getNodeLocation(node_);
  currentNode->astType = model::YamlAstNode::AstType::SEQUENCE;
  currentNode->symbolType = symbolType_;
  currentNode->entityHash = util::fnvHash(YAML::Dump(node_));
  currentNode->id = model::createIdentifier(*currentNode);
  _astNodes.push_back(currentNode);
}

model::Range YamlParser::getNodeLocation(YAML::Node& node_)
{
  model::Range location;
  location.start.line = node_.Mark().line;
  location.start.column = node_.Mark().column;
  std::string nodeValue = YAML::Dump(node_);
  auto count = std::count(nodeValue.begin(), nodeValue.end(), '\n');
  location.end.line = node_.Mark().line + count;

  if (count > 0)
  {
    auto pos = nodeValue.find_last_of('\n');
    location.end.column = nodeValue.size() - pos - 1;
  }
  else
  {
    location.end.column = node_.Mark().column + nodeValue.size();
  }

  return location;
}

bool YamlParser::isCIFile (std::string const& filename_, std::string const& ending_)
{
  if (filename_.length() >= ending_.length())
  {
    return (0 == filename_.compare (
            filename_.length() - ending_.length(), ending_.length(), ending_));
  }
  return false;
}

YamlParser::~YamlParser()
{
  (util::OdbTransaction(_ctx.db))([this]{
     util::persistAll(_astNodes, _ctx.db);
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
