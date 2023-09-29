#include <algorithm>
#include <boost/property_tree/ptree_fwd.hpp>
#include <iterator>
#include <stack>
#include <string>

#include <boost/property_tree/json_parser.hpp>

#include <cpplspservice/cpplspservice.h>
#include "language_types.h"

namespace cc
{
namespace service
{
namespace lsp
{

CppLspServiceHandler::CppLspServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _cppService(db_, datadir_, context_),
    _projectHandler(db_, datadir_, context_)
{
}

void CppLspServiceHandler::fillResponseTree(pt::ptree& responseTree_,
  const pt::ptree& params_,
  language::CppServiceHandler::ReferenceType refType_,
  bool canBeSingle_)
{
  TextDocumentPositionParams positionParams;
  positionParams.readNode(params_);

  std::vector<Location> locations = responseLocations(positionParams, refType_);

  if (canBeSingle_ && locations.size() == 1)
  {
    responseTree_.put_child("result", locations[0].createNode());
  }
  else if (locations.size() > 1)
  {
    pt::ptree resultNode;
    for (const Location &location : locations)
    {
      resultNode.push_back(std::make_pair("", location.createNode()));
    }
    responseTree_.put_child("result", resultNode);
  }
}

std::vector<Location> CppLspServiceHandler::responseLocations(
  const TextDocumentPositionParams& params_,
  language::CppServiceHandler::ReferenceType refType_)
{
  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return std::vector<Location>();

  cppPosition.file = std::to_string(file->id);
  cppPosition.pos.line = params_.position.line;
  cppPosition.pos.column = params_.position.character;

  _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

  std::vector<language::AstNodeInfo> nodeInfos;
  _cppService.getReferences(nodeInfos, astNodeInfo.id, refType_, {});

  std::vector<Location> locations(nodeInfos.size());
  std::transform(
    nodeInfos.begin(), nodeInfos.end(),
    locations.begin(),
    [&, this](const language::AstNodeInfo& nodeInfo)
    {
      std::string path = _transaction([&, this](){
        return _db->load<model::File>(std::stoull(nodeInfo.range.file))->path;
      });

      Location location;
      location.uri = path;
      location.range.start.line = nodeInfo.range.range.startpos.line;
      location.range.start.character = nodeInfo.range.range.startpos.column;
      location.range.end.line = nodeInfo.range.range.endpos.line;
      location.range.end.character = nodeInfo.range.range.endpos.column;

      return location;
    });

  return locations;
}

void CppLspServiceHandler::getSignature(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  TextDocumentPositionParams positionParams;
  positionParams.readNode(params_);

  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == positionParams.textDocument.uri);
  });

  if (file)
  {
    cppPosition.file = std::to_string(file->id);
    cppPosition.pos.line = positionParams.position.line;
    cppPosition.pos.column = positionParams.position.character;

    _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

    responseTree_.put("result", astNodeInfo.astNodeValue);
  }
}

void CppLspServiceHandler::getDefinition(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::DEFINITION);
}

void CppLspServiceHandler::getDeclaration(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::DECLARATION);
}

void CppLspServiceHandler::getImplementation(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::INHERIT_BY);
}

void CppLspServiceHandler::getReferences(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::USAGE,
    false); // the result must always be a vector
}

void CppLspServiceHandler::getParameters(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::PARAMETER);
}

void CppLspServiceHandler::getLocalVariables(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::LOCAL_VAR);
}

void CppLspServiceHandler::getOverridden(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::OVERRIDE);
}

void CppLspServiceHandler::getOverrider(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::OVERRIDDEN_BY);
}

void CppLspServiceHandler::getRead(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::READ);
}

void CppLspServiceHandler::getWrite(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::WRITE);
}

void CppLspServiceHandler::getMethods(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::METHOD);
}

void CppLspServiceHandler::getFriends(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::FRIEND);
}

void CppLspServiceHandler::getEnumConstants(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::ENUM_CONSTANTS);
}

void CppLspServiceHandler::getExpansion(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::EXPANSION);
}

void CppLspServiceHandler::getUndefinition(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::UNDEFINITION);
}

void CppLspServiceHandler::getThisCalls(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::THIS_CALLS);
}

void CppLspServiceHandler::getCallsOfThis(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::CALLS_OF_THIS);
}

void CppLspServiceHandler::getCallee(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::CALLEE);
}

void CppLspServiceHandler::getCaller(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::CALLER);
}

void CppLspServiceHandler::getVirtualCall(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::VIRTUAL_CALL);
}

void CppLspServiceHandler::getFunctionPointerCall(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::FUNC_PTR_CALL);
}

void CppLspServiceHandler::getAlias(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::ALIAS);
}

void CppLspServiceHandler::getImplements(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::INHERIT_FROM);
}

void CppLspServiceHandler::getDataMember(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::DATA_MEMBER);
}

void CppLspServiceHandler::getUnderlyingType(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  fillResponseTree(
    responseTree_,
    params_,
    language::CppServiceHandler::UNDERLYING_TYPE);
}

void CppLspServiceHandler::getDiagramTypes(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  DiagramTypeParams diagramTypeParams;
  diagramTypeParams.readNode(params_);

  std::vector<std::string> diagramTypesResult;
  if (!diagramTypeParams.position)
  {
    diagramTypesResult = fileDiagramTypes(diagramTypeParams);
  }
  else
  {
    diagramTypesResult = nodeDiagramTypes(diagramTypeParams);
  }

  pt::ptree resultNode;
  for (const std::string& diagramType : diagramTypesResult)
  {
    resultNode.push_back(std::make_pair("", pt::ptree(diagramType)));
  }
  responseTree_.put_child("result", resultNode);
}

std::vector<std::string> CppLspServiceHandler::fileDiagramTypes(
  const DiagramTypeParams& params_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return {};

  std::map<std::string, std::int32_t> result;
  _cppService.getFileDiagramTypes(result, std::to_string(file->id));

  std::vector<std::string> diagramTypes(result.size());
  std::transform(result.begin(), result.end(), diagramTypes.begin(),
    [](std::pair<std::string, std::int32_t> item)
    {
      return item.first;
    }
  );

  return diagramTypes;
}

std::vector<std::string> CppLspServiceHandler::nodeDiagramTypes(
  const DiagramTypeParams& params_)
{
  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return {};

  cppPosition.file = std::to_string(file->id);
  cppPosition.pos.line = params_.position->line;
  cppPosition.pos.column = params_.position->character;
  _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

  std::map<std::string, std::int32_t> result;
  _cppService.getDiagramTypes(result, astNodeInfo.id);

  std::vector<std::string> diagramTypes(result.size());
  std::transform(result.begin(), result.end(), diagramTypes.begin(),
    [](std::pair<std::string, std::int32_t> item)
    {
      return item.first;
    }
  );

  return diagramTypes;
}

void CppLspServiceHandler::getDiagram(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  DiagramParams diagramParams;
  diagramParams.readNode(params_);

  Diagram diagramResult;
  if (!diagramParams.position)
  {
    diagramResult = fileDiagram(diagramParams);
  }
  else
  {
    diagramResult = nodeDiagram(diagramParams);
  }

  responseTree_.put("result", diagramResult);
}

void CppLspServiceHandler::getModuleDiagram(
  pt::ptree& responseTree_,
  const pt::ptree& params_)
{
  getDiagram(responseTree_, params_);
}

Diagram CppLspServiceHandler::fileDiagram(
  const DiagramParams& params_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return std::string();

  const static std::map<std::string, std::int32_t> diagramTypes =
  {
    {"Internal architecture of this module",
      language::CppServiceHandler::SUBSYSTEM_DEPENDENCY},
    {"This module depends on",
      language::CppServiceHandler::EXTERNAL_DEPENDENCY},
    {"Users of this module", language::CppServiceHandler::EXTERNAL_USERS},
    {"Include Dependency", language::CppServiceHandler::INCLUDE_DEPENDENCY},
    {"Component Users", language::CppServiceHandler::COMPONENT_USERS},
    {"Interface Diagram", language::CppServiceHandler::INTERFACE},
  };

  auto diagramTypeIt = diagramTypes.find(params_.diagramType);
  if (diagramTypeIt == diagramTypes.end())
    return std::string();

  auto graph = _cppService.returnFileDiagram(
    std::to_string(file->id),
    diagramTypeIt->second);

  if (graph.nodeCount() != 0)
  {
    addPathToIdInFileDiagram(graph, std::to_string(file->id));
    return graph.output(util::Graph::SVG);
  }
  return std::string();
}

Diagram CppLspServiceHandler::nodeDiagram(
  const DiagramParams& params_)
{
  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return std::string();

  cppPosition.file = std::to_string(file->id);
  cppPosition.pos.line = params_.position->line;
  cppPosition.pos.column = params_.position->character;

  _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

  const static std::map<std::string, std::int32_t> diagramTypes =
  {
    {"Function call diagram", language::CppServiceHandler::FUNCTION_CALL},
    {"Detailed class diagram", language::CppServiceHandler::DETAILED_CLASS},
    {"Class collaboration diagram",
      language::CppServiceHandler::CLASS_COLLABORATION},
  };

  auto diagramTypeIt = diagramTypes.find(params_.diagramType);
  if (diagramTypeIt == diagramTypes.end())
    return std::string();

  auto graph = _cppService.returnDiagram(astNodeInfo.id, diagramTypeIt->second);

  if (graph.nodeCount() != 0)
  {
    addLocationToIdInDiagram(graph, astNodeInfo.id);
    return graph.output(util::Graph::SVG);
  }
  return std::string();
}

void CppLspServiceHandler::addLocationToIdInDiagram(
  util::Graph& graph_,
  const std::string& root_)
{
  std::stack<std::string> unvisited;
  std::map<std::string, bool> visited;
  unvisited.push(root_);

  while (!unvisited.empty())
  {
    std::string current = unvisited.top();
    unvisited.pop();
    std::vector<language::AstNodeInfo> nodes;
    _cppService.getReferences(
      nodes, current, language::CppServiceHandler::DEFINITION, {});
    language::AstNodeInfo& nodeInfo = nodes.front();
    if (visited.find(nodeInfo.id) == visited.end() || !visited[nodeInfo.id])
    {
      visited[nodeInfo.id] = true;
      for (const auto& child : graph_.getChildren(nodeInfo.id))
      {
        if (visited.find(child) == visited.end())
        {
          unvisited.push(child);
          visited[child] = false;
        }
      }
      for (const auto& parent : graph_.getParents(nodeInfo.id))
      {
        if (visited.find(parent) == visited.end())
        {
          unvisited.push(parent);
          visited[parent] = false;
        }
      }

      std::stringstream ss;
      ss<<_transaction([&, this](){
          return _db->load<model::File>(std::stoull(nodeInfo.range.file))->path;
        })
        <<';'
        <<nodeInfo.range.range.startpos.line
        <<';'
        <<nodeInfo.range.range.startpos.column;
      graph_.setNodeAttribute(nodeInfo.id, "id", ss.str(), true);
    }
  }
}

void CppLspServiceHandler::addPathToIdInFileDiagram(
  util::Graph& graph_,
  const std::string& root_)
{
  std::stack<std::string> unvisited;
  std::map<std::string, bool> visited;
  unvisited.push(root_);

  while (!unvisited.empty())
  {
    std::string current = unvisited.top();
    unvisited.pop();
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, current);
    if (visited.find(fileInfo.id) == visited.end() || !visited[fileInfo.id])
    {
      visited[fileInfo.id] = true;
      for (const auto& child : graph_.getChildren(fileInfo.id))
      {
        if (visited.find(child) == visited.end())
        {
          unvisited.push(child);
          visited[child] = false;
        }
      }
      for (const auto& parent : graph_.getParents(fileInfo.id))
      {
        if (visited.find(parent) == visited.end())
        {
          unvisited.push(parent);
          visited[parent] = false;
        }
      }
      graph_.setNodeAttribute(fileInfo.id, "id", fileInfo.path, true);
    }
  }
}

} // lsp
} // service
} // cc
