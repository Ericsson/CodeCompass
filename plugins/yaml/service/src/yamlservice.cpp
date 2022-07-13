#include <algorithm>
#include <regex>
#include <sstream>

#include <boost/filesystem.hpp>

#include <model/file.h>
#include <model/yamlastnode.h>
#include <service/yamlservice.h>

#include "yamlfilediagram.h"

namespace
{
  typedef odb::query<cc::model::YamlAstNode> AstQuery;
  typedef odb::result<cc::model::YamlAstNode> AstResult;
  typedef odb::query<cc::model::File> FileQuery;
  typedef odb::result<cc::model::File> FileResult;

  /**
 * This struct transforms a model::YamlAstNode to an AstNodeInfo Thrift
 * object.
 */
  struct CreateAstNodeInfo
  {
    typedef std::map<cc::model::YamlAstNodeId, std::vector<std::string>> TagMap;

    CreateAstNodeInfo(const TagMap& tags_ = {}) : _tags(tags_)
    {
    }

    /**
     * Returns the Thrift object for this Yaml AST node.
     */
    cc::service::language::AstNodeInfo operator()(
            const cc::model::YamlAstNode& astNode_)
    {
      cc::service::language::AstNodeInfo ret;

      ret.__set_id(std::to_string(astNode_.id));
      ret.__set_entityHash(astNode_.entityHash);
      ret.__set_astNodeType(cc::model::astTypeToString(astNode_.astType));
      ret.__set_symbolType(cc::model::symbolTypeToString(astNode_.symbolType));
      ret.__set_astNodeValue(astNode_.astValue);

      ret.range.range.startpos.line = astNode_.location.range.start.line;
      ret.range.range.startpos.column = astNode_.location.range.start.column;
      ret.range.range.endpos.line = astNode_.location.range.end.line;
      ret.range.range.endpos.column = astNode_.location.range.end.column;

      if (astNode_.location.file)
        ret.range.file = std::to_string(astNode_.location.file.object_id());

      TagMap::const_iterator it = _tags.find(astNode_.id);
      if (it != _tags.end())
        ret.__set_tags(it->second);

      return ret;
    }

    const std::map<cc::model::YamlAstNodeId, std::vector<std::string>>& _tags;
    std::shared_ptr<odb::database> _db;
  };
}

namespace cc
{
namespace service
{
namespace language
{

YamlServiceHandler::YamlServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
{
}

void YamlServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.emplace_back("YAML");
}

void YamlServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{
  return_ = _transaction([this, &astNodeId_]() {
    return CreateAstNodeInfo()(queryYamlAstNode(astNodeId_));
  });
}

void YamlServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_)
{
  _transaction([&, this](){
    //--- Query nodes at the given position ---//

    AstResult nodes = _db->query<model::YamlAstNode>(
      AstQuery::location.file == std::stoull(fpos_.file) &&
      // StartPos <= Pos
      ((AstQuery::location.range.start.line == fpos_.pos.line &&
        AstQuery::location.range.start.column <= fpos_.pos.column) ||
       AstQuery::location.range.start.line < fpos_.pos.line) &&
      // Pos < EndPos
      ((AstQuery::location.range.end.line == fpos_.pos.line &&
        AstQuery::location.range.end.column > fpos_.pos.column) ||
       AstQuery::location.range.end.line > fpos_.pos.line));

    if (!nodes.empty())
    {
      model::YamlAstNode temp = *(nodes.begin());

      return_ = _transaction([this,&temp](){
          return CreateAstNodeInfo()(temp);
      });
    }
  });
}

void YamlServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  return_ = _transaction([this, &astNodeId_](){
    model::YamlAstNode astNode = queryYamlAstNode(astNodeId_);

    if (astNode.location.file)
      return cc::util::textRange(
      astNode.location.file.load()->content.load()->content,
      astNode.location.range.start.line,
      astNode.location.range.start.column,
      astNode.location.range.end.line,
      astNode.location.range.end.column);

    return std::string();
  });
}

void YamlServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_) {}

void YamlServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  _transaction([&, this]() {
    model::YamlAstNode node = queryYamlAstNode(astNodeId_);

    return_["Name"] = node.astValue;
    return_["Symbol type"] = symbolTypeToString(node.symbolType);
    return_["Value type"] = astTypeToString(node.astType);
  });
}

void YamlServiceHandler::getDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_) {}

void YamlServiceHandler::getDiagram(
        std::string& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t diagramId_) {}

void YamlServiceHandler::getDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_) {}

void YamlServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
    FileQuery::id == std::stoull(fileId_));
  });

  if (file)
  {
    if (file->type == "YAML")
    {
      return_["File Info"]     = YAMLFILEINFO;
      return_["Root keys"]     = ROOTKEYS;
    }
  }
}

void YamlServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_)
{
  YamlFileDiagram diagram(_db, _datadir, _context);
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  switch (diagramId_)
  {
    case ROOTKEYS:
      diagram.getYamlFileDiagram(graph, fileId_);
      break;
    case YAMLFILEINFO:
      diagram.getYamlFileInfo(graph, fileId_);
      break;
  }

  return_ = graph.output(util::Graph::DOT);
}

void YamlServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_) {}

void YamlServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId) {}

void YamlServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_) {}

std::int32_t YamlServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_) {}

void YamlServiceHandler::getReferencesInFile(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const core::FileId& fileId_,
        const std::vector<std::string>& tags_) {}

void YamlServiceHandler::getReferencesPage(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::int32_t pageSize_,
        const std::int32_t pageNo_) {}

void YamlServiceHandler::getFileReferenceTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_) {}

void YamlServiceHandler::getFileReferences(
        std::vector<AstNodeInfo>& return_,
        const core::FileId& fileId_,
        const std::int32_t referenceId_) {}

std::int32_t YamlServiceHandler::getFileReferenceCount(
        const core::FileId& fileId_,
        const std::int32_t referenceId_) {}

void YamlServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileRange& range_)
{
  std::vector<std::string> content;

  _transaction([&, this]()
  {
    //--- Load the file content and break it into lines ---//

    model::FilePtr file = _db->query_one<model::File>(
            FileQuery::id == std::stoull(range_.file));

    if (!file || !file->content.load())
      return;

    std::istringstream s(file->content->content);
    std::string line;
    while (std::getline(s, line))
      content.push_back(line);

    //--- Iterate over AST node elements ---//

    for (const model::YamlAstNode& node : _db->query<model::YamlAstNode>(
            AstQuery::location.file == std::stoull(range_.file) &&
            AstQuery::location.range.start.line >= range_.range.startpos.line &&
            AstQuery::location.range.end.line < range_.range.endpos.line &&
            AstQuery::location.range.end.line != model::Position::npos))
    {
      if (node.astValue.empty())
        continue;

      for (std::size_t i = node.location.range.start.line - 1;
           i < node.location.range.end.line && i < content.size();
           ++i)
      {
        SyntaxHighlight syntax;
        syntax.range.startpos.line = i + 1;
        syntax.range.startpos.column = node.location.range.start.column;//ri->position() + 1;
        syntax.range.endpos.line = i + 1;
        syntax.range.endpos.column =
                syntax.range.startpos.column + node.astValue.length() + 1;

        std::string symbolClass =
                "cm-" + model::symbolTypeToString(node.symbolType);
        syntax.className = symbolClass;

        return_.push_back(std::move(syntax));
      }
    }
  });
}

model::YamlAstNode YamlServiceHandler::queryYamlAstNode(
  const core::AstNodeId &astNodeId_)
{
  return _transaction([&, this](){
    model::YamlAstNode node;

    if (!_db->find(std::stoull(astNodeId_), node))
    {
      core::InvalidId ex;
      ex.__set_msg("Invalid YamlAstNode ID");
      ex.__set_nodeid(astNodeId_);
      throw ex;
    }

    return node;
  });
}

}
}
}
