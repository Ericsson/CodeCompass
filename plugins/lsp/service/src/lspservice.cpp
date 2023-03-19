#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <iterator>

#include <language_types.h>

#include <lspservice/lspservice.h>

namespace cc
{ 
namespace service
{
namespace lsp
{

std::unordered_map<std::string, LspServiceHandler::LspMethod> LspServiceHandler::_methodMap = {
  { "textDocument/definition",     LspServiceHandler::LspMethod::Definition },
  { "textDocument/implementation", LspServiceHandler::LspMethod::Implementation },
  { "textDocument/references",     LspServiceHandler::LspMethod::References },
  { "diagram/diagramTypes",        LspServiceHandler::LspMethod::DiagramTypes },
  { "diagram/diagram",             LspServiceHandler::LspMethod::Diagram},
};

LspServiceHandler::LspServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _cppService(db_, datadir_, context_)
{
}

void LspServiceHandler::getLspResponse(std::string& _return, const std::string& request)
{
  pt::ptree responseTree;
  responseTree.put("jsonrpc", "2.0");

  try
  {
    pt::ptree requestTree;
    std::stringstream requestStream(request);
    pt::read_json(requestStream, requestTree);

    std::string requestId = requestTree.get<std::string>("id");
    responseTree.put("id", requestId);

    std::string method = requestTree.get<std::string>("method");
    pt::ptree& params = requestTree.get_child("params");

    switch (parseMethod(method))
    {
      case LspMethod::Definition:
      {
        TextDocumentPositionParams gotoDefParams;
        gotoDefParams.readNode(params);

        std::vector<Location> gotoDefLocations =
          definition(gotoDefParams);

        if (gotoDefLocations.size() == 1)
        {
          responseTree.put_child("result", gotoDefLocations[0].createNode());
        }
        else if (gotoDefLocations.size() > 1)
        {
          pt::ptree resultNode;
          for (const Location &location : gotoDefLocations)
          {
            resultNode.push_back(std::make_pair("", location.createNode()));
          }
          responseTree.put_child("result", resultNode);
        }
        break;
      }
      case LspMethod::Implementation:
      {
        TextDocumentPositionParams implementationParams;
        implementationParams.readNode(params);

        std::vector<Location> implementationLocations =
          implementation(implementationParams);

        if (implementationLocations.size() == 1)
        {
          responseTree.put_child("result", implementationLocations[0].createNode());
        }
        else if (implementationLocations.size() > 1)
        {
          pt::ptree resultNode;
          for (const Location &location : implementationLocations)
          {
            resultNode.push_back(std::make_pair("", location.createNode()));
          }
          responseTree.put_child("result", resultNode);
        }
        break;
      }
      case LspMethod::References:
      {
        ReferenceParams refParams;
        refParams.readNode(params);

        std::vector<Location> refLocations = references(refParams);

        pt::ptree resultNode;
        for (const Location& location : refLocations)
        {
          resultNode.push_back(std::make_pair("", location.createNode()));
        }
        responseTree.put_child("result", resultNode);
        break;
      }
      case LspMethod::DiagramTypes:
      {
        DiagramTypeParams diagramTypeParams;
        diagramTypeParams.readNode(params);

        CompletionList diagramTypesResult;
        if (!diagramTypeParams.position)
        {
          diagramTypesResult = fileDiagramTypes(diagramTypeParams);
        }
        else
        {
          diagramTypesResult = nodeDiagramTypes(diagramTypeParams);
        }

        responseTree.put_child("result", diagramTypesResult.createNode());
        break;
      }
      case LspMethod::Diagram:
      {
        DiagramParams diagramParams;
        diagramParams.readNode(params);

        Diagram diagramResult;
        if (!diagramParams.position)
        {
          diagramResult = fileDiagram(diagramParams);
        }
        else
        {
          diagramResult = nodeDiagram(diagramParams);
        }

        responseTree.put("result", diagramResult);
        break;
      }
      default:
      {
        LOG(warning) << "[LSP] Unsupported method: '" << method << "'";

        ResponseError error;
        error.code = ErrorCode::MethodNotFound;
        error.message = std::string("Unsupported method: ").append(method);
        responseTree.put_child("error", error.createNode());
      }
    }
  }
  catch (const pt::ptree_error& ex)
  {
    LOG(warning) << ex.what();

    ResponseError error;
    error.code = ErrorCode::ParseError;
    error.message = std::string("JSON RPC parsing error: ").append(ex.what());
    responseTree.put_child("error", error.createNode());
  }
  catch (const std::exception& ex)
  {
    LOG(warning) << ex.what();

    ResponseError error;
    error.code = ErrorCode::InternalError;
    error.message = ex.what();
    responseTree.put_child("error", error.createNode());
  }
  catch (...)
  {
    LOG(warning) << "Unknown exception has been caught";

    ResponseError error;
    error.code = ErrorCode::UnknownError;
    responseTree.put_child("error", error.createNode());
  }

  std::stringstream responseStream;
  pt::write_json(responseStream, responseTree);
  _return = responseStream.str();
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

std::vector<Location> LspServiceHandler::implementation(
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

LspServiceHandler::LspMethod LspServiceHandler::parseMethod(const std::string& method)
{
  auto it = _methodMap.find(method);
  if (it != _methodMap.end())
    return it->second;
  return LspMethod::Unknown;
}

} // lsp
} // service
} // cc
