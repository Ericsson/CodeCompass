#include <algorithm>
#include <iterator>

#include <language_types.h>

#include <lspservice/lspservice.h>

namespace cc
{ 
namespace service
{
namespace lsp
{

LspServiceHandler::LspServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _context(context_),
    _cppService(db_, datadir_, context_)
{
}

std::vector<Location> LspServiceHandler::definition(
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

std::vector<Location> LspServiceHandler::references(
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

CompletionList LspServiceHandler::fileDiagramTypes(
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

CompletionList LspServiceHandler::nodeDiagramTypes(
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

Diagram LspServiceHandler::fileDiagram(
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

Diagram LspServiceHandler::nodeDiagram(
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
