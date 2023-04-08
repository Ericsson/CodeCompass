#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <iterator>

#include <cpplspservice/cpplspservice.h>

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
    _cppService(db_, datadir_, context_)
{
}

void CppLspServiceHandler::getDefinition(pt::ptree& responseTree_, const pt::ptree& params_)
{
  TextDocumentPositionParams gotoDefParams;
  gotoDefParams.readNode(params_);

  std::vector<Location> gotoDefLocations =
    definition(gotoDefParams);

  if (gotoDefLocations.size() == 1)
  {
    responseTree_.put_child("result", gotoDefLocations[0].createNode());
  }
  else if (gotoDefLocations.size() > 1)
  {
    pt::ptree resultNode;
    for (const Location &location : gotoDefLocations)
    {
      resultNode.push_back(std::make_pair("", location.createNode()));
    }
    responseTree_.put_child("result", resultNode);
  }
}

std::vector<Location> CppLspServiceHandler::definition(
  const TextDocumentPositionParams& params_)
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

  std::vector<language::AstNodeInfo> definitionInfo;
  _cppService.getReferences(definitionInfo, astNodeInfo.id, language::CppServiceHandler::DEFINITION, {});

  std::vector<Location> definitionLocations(definitionInfo.size());
  std::transform(
    definitionInfo.begin(), definitionInfo.end(),
    definitionLocations.begin(),
    [&, this](const language::AstNodeInfo& definition)
    {
      std::string path = _transaction([&, this](){
        return _db->load<model::File>(std::stoull(definition.range.file))->path;
      });

      Location location;
      location.uri = path;
      location.range.start.line = definition.range.range.startpos.line;
      location.range.start.character = definition.range.range.startpos.column;
      location.range.end.line = definition.range.range.endpos.line;
      location.range.end.character = definition.range.range.endpos.column;

      return location;
    });

  return definitionLocations;
}

void CppLspServiceHandler::getImplementation(pt::ptree& responseTree_, const pt::ptree& params_)
{
  TextDocumentPositionParams implementationParams;
  implementationParams.readNode(params_);

  std::vector<Location> implementationLocations =
    implementation(implementationParams);

  if (implementationLocations.size() == 1)
  {
    responseTree_.put_child("result", implementationLocations[0].createNode());
  }
  else if (implementationLocations.size() > 1)
  {
    pt::ptree resultNode;
    for (const Location &location : implementationLocations)
    {
      resultNode.push_back(std::make_pair("", location.createNode()));
    }
    responseTree_.put_child("result", resultNode);
  }
}

std::vector<Location> CppLspServiceHandler::implementation(
  const TextDocumentPositionParams& params_)
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

  std::vector<language::AstNodeInfo> implementationInfo;
  _cppService.getReferences(implementationInfo, astNodeInfo.id, language::CppServiceHandler::INHERIT_BY, {});

  std::vector<Location> implementationLocations(implementationInfo.size());
  std::transform(
    implementationInfo.begin(), implementationInfo.end(),
    implementationLocations.begin(),
    [&, this](const language::AstNodeInfo& definition)
    {
      std::string path = _transaction([&, this](){
        return _db->load<model::File>(std::stoull(definition.range.file))->path;
      });

      Location location;
      location.uri = path;
      location.range.start.line = definition.range.range.startpos.line;
      location.range.start.character = definition.range.range.startpos.column;
      location.range.end.line = definition.range.range.endpos.line;
      location.range.end.character = definition.range.range.endpos.column;

      return location;
    });

  return implementationLocations;
}

void CppLspServiceHandler::getReferences(pt::ptree& responseTree_, const pt::ptree& params_)
{
  ReferenceParams refParams;
  refParams.readNode(params_);

  std::vector<Location> refLocations = references(refParams);

  pt::ptree resultNode;
  for (const Location& location : refLocations)
  {
    resultNode.push_back(std::make_pair("", location.createNode()));
  }
  responseTree_.put_child("result", resultNode);
}

std::vector<Location> CppLspServiceHandler::references(
  const ReferenceParams& params_)
{
  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this]()
                                     {
                                       return _db->query_one<model::File>(
                                         odb::query<model::File>::path == params_.textDocument.uri);
                                     });

  if (!file)
    return std::vector<Location>();

  cppPosition.file = std::to_string(file->id);
  cppPosition.pos.line = params_.position.line;
  cppPosition.pos.column = params_.position.character;

  _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

  //--- Usages ---//
  std::vector<language::AstNodeInfo> usageInfo;
  _cppService.getReferences(usageInfo, astNodeInfo.id, language::CppServiceHandler::USAGE, {});

  std::vector<Location> usageLocations(usageInfo.size());
  std::transform(
    usageInfo.begin(), usageInfo.end(),
    usageLocations.begin(),
    [&, this](const language::AstNodeInfo &usage)
    {
      std::string path = _transaction([&, this]()
                                      {
                                        return _db->load<model::File>(std::stoull(usage.range.file))->path;
                                      });

      Location location;
      location.uri = path;
      location.range.start.line = usage.range.range.startpos.line;
      location.range.start.character = usage.range.range.startpos.column;
      location.range.end.line = usage.range.range.endpos.line;
      location.range.end.character = usage.range.range.endpos.column;

      return location;
    });

  if (!params_.context.includeDeclaration)
  {
    return usageLocations;
  }

  //--- Declarations ---//
  std::vector<language::AstNodeInfo> declarationInfo;
  _cppService.getReferences(declarationInfo, astNodeInfo.id, language::CppServiceHandler::DECLARATION, {});

  std::vector<Location> declarationLocations(declarationInfo.size());
  std::transform(
    declarationInfo.begin(), declarationInfo.end(),
    declarationLocations.begin(),
    [&, this](const language::AstNodeInfo &declaration)
    {
      std::string path = _transaction([&, this]()
                                      {
                                        return _db->load<model::File>(std::stoull(declaration.range.file))->path;
                                      });

      Location location;
      location.uri = path;
      location.range.start.line = declaration.range.range.startpos.line;
      location.range.start.character = declaration.range.range.startpos.column;
      location.range.end.line = declaration.range.range.endpos.line;
      location.range.end.character = declaration.range.range.endpos.column;

      return location;
    });

  usageLocations.insert(usageLocations.end(),
    std::make_move_iterator(declarationLocations.begin()),
    std::make_move_iterator(declarationLocations.end()));
  return usageLocations;
}

void CppLspServiceHandler::getDiagramTypes(pt::ptree& responseTree_, const pt::ptree& params_)
{
  DiagramTypeParams diagramTypeParams;
  diagramTypeParams.readNode(params_);

  CompletionList diagramTypesResult;
  if (!diagramTypeParams.position)
  {
    diagramTypesResult = fileDiagramTypes(diagramTypeParams);
  }
  else
  {
    diagramTypesResult = nodeDiagramTypes(diagramTypeParams);
  }

  responseTree_.put_child("result", diagramTypesResult.createNode());
}

CompletionList CppLspServiceHandler::fileDiagramTypes(
  const DiagramTypeParams& params_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return CompletionList();

  std::map<std::string, std::int32_t> result;
  _cppService.getFileDiagramTypes(result, std::to_string(file->id));

  std::vector<CompletionItem> diagramTypes(result.size());
  std::transform(result.begin(), result.end(), diagramTypes.begin(),
    [](std::pair<std::string, std::int32_t> item)
    {
      CompletionItem ci;
      ci.label = item.first;
      ci.data = std::to_string(item.second);
      return ci;
    }
  );

  CompletionList list;
  list.isIncomplete = false;
  list.items = diagramTypes;
  return list;
}

CompletionList CppLspServiceHandler::nodeDiagramTypes(
  const DiagramTypeParams& params_)
{
  language::AstNodeInfo astNodeInfo;
  core::FilePosition cppPosition;

  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return CompletionList();

  cppPosition.file = std::to_string(file->id);
  cppPosition.pos.line = params_.position->line;
  cppPosition.pos.column = params_.position->character;
  _cppService.getAstNodeInfoByPosition(astNodeInfo, cppPosition);

  std::map<std::string, std::int32_t> result;
  _cppService.getDiagramTypes(result, astNodeInfo.id);

  std::vector<CompletionItem> diagramTypes(result.size());
  std::transform(result.begin(), result.end(), diagramTypes.begin(),
    [](std::pair<std::string, std::int32_t> item)
    {
      CompletionItem ci;
      ci.label = item.first;
      ci.data = std::to_string(item.second);
      return ci;
    }
  );

  CompletionList list;
  list.isIncomplete = false;
  list.items = diagramTypes;
  return list;
}

void CppLspServiceHandler::getDiagram(pt::ptree& responseTree_, const pt::ptree& params_)
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

Diagram CppLspServiceHandler::fileDiagram(
  const DiagramParams& params_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<model::File>::path == params_.textDocument.uri);
  });

  if (!file)
    return std::string();

  std::map<std::string, std::int32_t> diagramTypes;
  _cppService.getFileDiagramTypes(diagramTypes, std::to_string(file->id));

  auto diagramTypeIt = diagramTypes.find(params_.diagramType);
  if(diagramTypeIt == diagramTypes.end())
    return std::string();

  Diagram diagram;
  _cppService.getFileDiagram(diagram, std::to_string(file->id), diagramTypeIt->second);
  return diagram;
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

  std::map<std::string, std::int32_t> diagramTypes;
  _cppService.getDiagramTypes(diagramTypes, astNodeInfo.id);

  auto diagramTypeIt = diagramTypes.find(params_.diagramType);
  if(diagramTypeIt == diagramTypes.end())
    return std::string();

  Diagram diagram;
  _cppService.getDiagram(diagram, astNodeInfo.id, diagramTypeIt->second);

  return diagram;
}

} // lsp
} // service
} // cc
