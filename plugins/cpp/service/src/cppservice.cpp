#include <algorithm>
#include <queue>

#include <util/util.h>

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/cppvariable.h>
#include <model/cppvariable-odb.hxx>
#include <model/cpptype.h>
#include <model/cpptype-odb.hxx>
#include <model/cpptypedef.h>
#include <model/cpptypedef-odb.hxx>
#include <model/cppinheritance.h>
#include <model/cppinheritance-odb.hxx>
#include <model/cppfriendship.h>
#include <model/cppfriendship-odb.hxx>
#include <model/cppenum.h>
#include <model/cppenum-odb.hxx>

#include <service/cppservice.h>

#include "diagram.h"

namespace
{
  typedef odb::query<cc::model::CppAstNode> AstQuery;
  typedef odb::result<cc::model::CppAstNode> AstResult;
  typedef odb::query<cc::model::CppFunction> FuncQuery;
  typedef odb::result<cc::model::CppFunction> FuncResult;
  typedef odb::query<cc::model::CppRelation> RelQuery;
  typedef odb::result<cc::model::CppRelation> RelResult;
  typedef odb::query<cc::model::CppVariable> VarQuery;
  typedef odb::result<cc::model::CppVariable> VarResult;
  typedef odb::query<cc::model::CppType> TypeQuery;
  typedef odb::result<cc::model::CppType> TypeResult;
  typedef odb::query<cc::model::CppTypedef> TypedefQuery;
  typedef odb::result<cc::model::CppTypedef> TypedefResult;
  typedef odb::query<cc::model::CppMemberType> MemTypeQuery;
  typedef odb::result<cc::model::CppMemberType> MemTypeResult;
  typedef odb::query<cc::model::CppInheritance> InhQuery;
  typedef odb::result<cc::model::CppInheritance> InhResult;
  typedef odb::query<cc::model::CppFriendship> FriendQuery;
  typedef odb::result<cc::model::CppFriendship> FriendResult;
  typedef odb::query<cc::model::CppEnum> EnumQuery;
  typedef odb::result<cc::model::CppEnum> EnumResult;
  typedef odb::query<cc::model::CppEnumConstant> EnumConstQuery;
  typedef odb::result<cc::model::CppEnumConstant> EnumConstResult;

  /**
   * This struct transforms a model::CppAstNode to an AstNodeInfo Thrift
   * object.
   */
  struct CreateAstNodeInfo
  {
    typedef std::map<cc::model::CppAstNodeId, std::vector<std::string>> TagMap;

    CreateAstNodeInfo(
      std::shared_ptr<odb::database> db_,
      const TagMap& tags_ = {}) : _db(db_), _tags(tags_)
    {
    }

    cc::service::language::AstNodeInfo operator()(
      const cc::model::CppAstNode& astNode_)
    {
      cc::service::language::AstNodeInfo ret;

      ret.__set_id(std::to_string(astNode_.id));
      ret.__set_mangledNameHash(astNode_.mangledNameHash);
      ret.__set_astNodeType(cc::model::astTypeToString(astNode_.astType));
      ret.__set_symbolType(cc::model::symbolTypeToString(astNode_.symbolType));
      ret.__set_astNodeValue(astNode_.astValue);

      ret.range.range.startpos.line = astNode_.location.range.start.line;
      ret.range.range.startpos.column = astNode_.location.range.start.column;
      ret.range.range.endpos.line = astNode_.location.range.end.line;
      ret.range.range.endpos.column = astNode_.location.range.end.column;

      if (astNode_.location.file)
      {
        ret.range.file = std::to_string(astNode_.location.file.object_id());

        (cc::util::OdbTransaction(_db))([&ret, &astNode_](){
          ret.__set_srcText(cc::util::textRange(
            astNode_.location.file.load()->content.load()->content,
            astNode_.location.range.start.line,
            astNode_.location.range.start.column,
            astNode_.location.range.end.line,
            astNode_.location.range.end.column));
        });
      }

      TagMap::const_iterator it = _tags.find(astNode_.id);
      if (it != _tags.end())
        ret.__set_tags(it->second);

      return ret;
    }

    const std::map<cc::model::CppAstNodeId, std::vector<std::string>>& _tags;
    std::shared_ptr<odb::database> _db;
  };
}

namespace cc
{
namespace service
{
namespace language
{

CppServiceHandler::CppServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const boost::program_options::variables_map& config_)
    : _db(db_), _transaction(db_), _datadir(datadir_), _config(config_)
{
}

void CppServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("CPP");
}

void CppServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{
  return_ = CreateAstNodeInfo(_db)(queryCppAstNode(astNodeId_));
}

void CppServiceHandler::getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_)
{
  // TODO
}

void CppServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_)
{
  _transaction([&, this](){
    //--- Query nodes at the given position ---//

    AstResult nodes = _db->query<model::CppAstNode>(
      AstQuery::location.file == std::stoull(fpos_.file) &&
      // StartPos <= Pos
      ((AstQuery::location.range.start.line == fpos_.pos.line &&
        AstQuery::location.range.start.column <= fpos_.pos.column) ||
       AstQuery::location.range.start.line < fpos_.pos.line) &&
      // Pos < EndPos
      ((AstQuery::location.range.end.line == fpos_.pos.line &&
        AstQuery::location.range.end.column > fpos_.pos.column) ||
       AstQuery::location.range.end.line > fpos_.pos.line));

    //--- Select innermost clickable node ---//

    model::Range minRange(model::Position(0, 0), model::Position());
    model::CppAstNode min;

    for (const model::CppAstNode& node : nodes)
    {
      // TODO: Remove ugly hack and use CppAstNode::visibleInSourceCode when it
      // will be available.
      if (node.symbolType == model::CppAstNode::SymbolType::Macro)
      {
        min = node;
        break;
      }

      if (node.visibleInSourceCode && node.location.range < minRange)
      {
        min = node;
        minRange = node.location.range;
      }
    }

    return_ = CreateAstNodeInfo(_db, getTags({min}))(min);
  });
}

void CppServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  _transaction([&, this](){
    model::CppAstNode node = queryCppAstNode(astNodeId_);
    std::vector<model::CppAstNode> defs = queryDefinitions(astNodeId_);

    if (defs.empty())
      return;

    switch (node.symbolType)
    {
      case model::CppAstNode::SymbolType::Variable:
      {
        VarResult variables = _db->query<model::CppVariable>(
          VarQuery::mangledNameHash == defs.front().mangledNameHash);
        model::CppVariable variable = *variables.begin();

        return_["Name"] = variable.name;
        return_["Qualified name"] = variable.qualifiedName;
        return_["Type"] = variable.qualifiedType;
        break;
      }

      case model::CppAstNode::SymbolType::Function:
      {
        FuncResult functions = _db->query<model::CppFunction>(
          FuncQuery::mangledNameHash == defs.front().mangledNameHash);
        model::CppFunction function = *functions.begin();

        return_["Name"] = function.qualifiedName.substr(
          function.qualifiedName.find_last_of(':') + 1);
        return_["Qualified name"] = function.qualifiedName;
        return_["Signature"] = function.name;

        break;
      }

      case model::CppAstNode::SymbolType::Type:
      {
        TypeResult types = _db->query<model::CppType>(
          TypeQuery::mangledNameHash == defs.front().mangledNameHash);
        model::CppType type = *types.begin();

        if (type.isAbstract)
          return_["Abstract type"] = "true";
        if (type.isPOD)
          return_["POD type"] = "true";

        return_["Name"] = type.name;
        return_["Qualified name"] = type.qualifiedName;

        break;
      }

      case model::CppAstNode::SymbolType::Typedef:
      {
        TypedefResult types = _db->query<model::CppTypedef>(
          TypedefQuery::mangledNameHash == defs.front().mangledNameHash);
        model::CppTypedef type = *types.begin();

        return_["Name"] = type.name;
        return_["Qualified name"] = type.qualifiedName;

        break;
      }

      case model::CppAstNode::SymbolType::EnumConstant:
      {
        EnumConstResult enumConsts
          = _db->query<model::CppEnumConstant>(
              EnumConstQuery::mangledNameHash == defs.front().mangledNameHash);
        model::CppEnumConstant enumConst = *enumConsts.begin();

        return_["Name"] = enumConst.name;
        return_["Qualified name"] = enumConst.qualifiedName;
        return_["Value"] = std::to_string(enumConst.value);
      }
    }
  });
}

void CppServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  return_["Definition"]                = DEFINITION;
  return_["Declaration"]               = DECLARATION;
  return_["Usage"]                     = USAGE;

  switch (node.symbolType)
  {
    case model::CppAstNode::SymbolType::Function:
      return_["This calls"]            = THIS_CALLS;
      return_["Callee"]                = CALLEE;
      return_["Caller"]                = CALLER;
      return_["Virtual call"]          = VIRTUAL_CALL;
      return_["Function pointer call"] = FUNC_PTR_CALL;
      return_["Parameters"]            = PARAMETER;
      return_["Local variables"]       = LOCAL_VAR;
      return_["Overrides"]             = OVERRIDE;
      return_["Overridden by"]         = OVERRIDDEN_BY;
      break;

    case model::CppAstNode::SymbolType::Variable:
      return_["Reads"]                 = READ;
      return_["Writes"]                = WRITE;
      return_["Type"]                  = TYPE;
      break;

    case model::CppAstNode::SymbolType::Type:
      return_["Aliases"]               = ALIAS;
      return_["Inherits from"]         = INHERIT_FROM;
      return_["Inherited by"]          = INHERIT_BY;
      return_["Data member"]           = DATA_MEMBER;
      return_["Method"]                = METHOD;
      return_["Friends"]               = FRIEND;
      return_["Usage"]                 = USAGE;
      break;

    case model::CppAstNode::SymbolType::Typedef:
      return_["Underlying type"]       = UNDERLYING_TYPE;
      break;

    case model::CppAstNode::SymbolType::Enum:
      return_["Enum constants"]        = ENUM_CONSTANTS;
      break;
  }
}

void CppServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_)
{
  std::map<model::CppAstNodeId, std::vector<std::string>> tags;
  std::vector<model::CppAstNode> nodes;
  model::CppAstNode node;

  _transaction([&, this](){
    switch (referenceId_)
    {
      case DEFINITION:
        nodes = queryDefinitions(astNodeId_);
        break;

      case DECLARATION:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Declaration &&
          AstQuery::visibleInSourceCode == true);
        break;

      case USAGE: // TODO: Filter by tags
        nodes = queryCppAstNodes(astNodeId_);
        break;

      case THIS_CALLS:
        nodes = queryCalls(astNodeId_);
        break;

      case CALLS_OF_THIS:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Usage);
        break;

      case CALLEE:
        for (const model::CppAstNode& call : queryCalls(astNodeId_))
        {
          core::AstNodeId astNodeId = std::to_string(call.id);
          std::vector<model::CppAstNode> defs = queryDefinitions(astNodeId);
          nodes.insert(nodes.end(), defs.begin(), defs.end());
        }

        std::sort(nodes.begin(), nodes.end());
        nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

        break;

      case CALLER:
        for (const model::CppAstNode& astNode : queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Usage))
        {
          const model::Position& start = astNode.location.range.start;
          const model::Position& end   = astNode.location.range.end;

          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::astType    == model::CppAstNode::AstType::Definition &&
            AstQuery::symbolType == model::CppAstNode::SymbolType::Function &&
            // Same file
            AstQuery::location.file == astNode.location.file.object_id() &&
            // StartPos >= Pos
            ((AstQuery::location.range.start.line == start.line &&
              AstQuery::location.range.start.column <= start.column) ||
             AstQuery::location.range.start.line < start.line) &&
            // Pos > EndPos
            ((AstQuery::location.range.end.line == end.line &&
              AstQuery::location.range.end.column > end.column) ||
             AstQuery::location.range.end.line > end.line));

          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        std::sort(nodes.begin(), nodes.end());
        nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

        break;

      case VIRTUAL_CALL:
      {
        nodes = queryCppAstNodes(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::VirtualCall);

        for (const model::CppAstNode& node : queryOverrides(astNodeId_, true))
        {
          core::AstNodeId astNodeId = std::to_string(node.id);
          std::vector<model::CppAstNode> calls = queryCppAstNodes(astNodeId,
            AstQuery::astType == model::CppAstNode::AstType::VirtualCall);

          nodes.insert(nodes.end(), calls.begin(), calls.end());
        }

        std::sort(nodes.begin(), nodes.end());
        nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

        break;
      }

      case FUNC_PTR_CALL:
      {
        node = queryCppAstNode(astNodeId_);

        std::unordered_set<std::uint64_t> fptrCallers
          = transitiveClosureOfRel(
              model::CppRelation::Kind::Assign,
              node.mangledNameHash,
              true);

        for (std::uint64_t mangledNameHash : fptrCallers)
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == mangledNameHash &&
            AstQuery::astType == model::CppAstNode::AstType::Usage);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;
      }

      case PARAMETER:
      {
        node = queryCppAstNode(astNodeId_);

        FuncResult functions = _db->query<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);
        model::CppFunction function = *functions.begin();

        for (auto var : function.parameters)
          nodes.push_back(queryCppAstNode(
            std::to_string(var.load()->astNodeId)));

        break;
      }

      case LOCAL_VAR:
      {
        node = queryCppAstNode(astNodeId_);

        FuncResult functions = _db->query<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);
        model::CppFunction function = *functions.begin();

        for (auto var : function.locals)
          nodes.push_back(queryCppAstNode(
            std::to_string(var.load()->astNodeId)));

        break;
      }

      case OVERRIDE:
        nodes = queryOverrides(astNodeId_, true);
        break;

      case OVERRIDDEN_BY:
        nodes = queryOverrides(astNodeId_, false);
        break;

      case READ:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Read);

        break;

      case WRITE:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Write);

        break;

      case ALIAS:
      {
        node = queryCppAstNode(astNodeId_);

        TypedefResult result = _db->query<model::CppTypedef>(
          TypedefQuery::typeHash == node.mangledNameHash);

        for (const auto& typeDef : result)
          nodes.push_back(queryCppAstNode(std::to_string(typeDef.astNodeId)));

        break;
      }

      case TYPE:
      {
        node = queryCppAstNode(astNodeId_);

        VarResult varNodes = _db->query<cc::model::CppVariable>(
          VarQuery::mangledNameHash == node.mangledNameHash);

        const model::CppVariable& variable = *varNodes.begin();

        TypeResult result = _db->query<model::CppType>(
          TypeQuery::mangledNameHash == variable.typeHash);

        for (const model::CppType& type : result)
        {
          std::vector<model::CppAstNode> defs =
            queryDefinitions(std::to_string(type.astNodeId));
          nodes.insert(nodes.end(), defs.begin(), defs.end());
        }

        break;
      }

      case INHERIT_FROM:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppInheritance& inh :
          _db->query<model::CppInheritance>(
            InhQuery::derived == node.mangledNameHash)) // TODO: Filter by tags
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == inh.base &&
            AstQuery::astType == model::CppAstNode::AstType::Definition);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;

      case INHERIT_BY:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppInheritance& inh :
          _db->query<model::CppInheritance>(
            InhQuery::base == node.mangledNameHash )) // TODO: Filter by tags
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == inh.derived &&
            AstQuery::astType == model::CppAstNode::AstType::Definition);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;

      case DATA_MEMBER:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppMemberType& mem : _db->query<model::CppMemberType>(
          MemTypeQuery::typeHash == node.mangledNameHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Field))
          // TODO: Filter by tags
        {
          model::CppAstNodePtr astNode = mem.memberAstNode.load();

          if (astNode->location.range.end.line != model::Position::npos)
            nodes.push_back(*astNode);
        }

        break;

      case METHOD:
      {
        node = queryCppAstNode(astNodeId_);

        for (const model::CppMemberType& mem : _db->query<model::CppMemberType>(
          MemTypeQuery::typeHash == node.mangledNameHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Method))
          // TODO: Filter by tags
        {
          model::CppAstNodePtr astNode = mem.memberAstNode.load();
          nodes.push_back(*astNode);
        }

        break;
      }

      case FRIEND:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppFriendship& fr : _db->query<model::CppFriendship>(
          FriendQuery::target == node.mangledNameHash))
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == fr.theFriend &&
            AstQuery::astType == model::CppAstNode::AstType::Definition);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;

      case UNDERLYING_TYPE:
      {
        node = queryCppAstNode(astNodeId_);

        TypedefResult types = _db->query<model::CppTypedef>(
          TypedefQuery::mangledNameHash == node.mangledNameHash);
        model::CppTypedef type = *types.begin();

        AstResult result = _db->query<model::CppAstNode>(
          AstQuery::mangledNameHash == type.typeHash &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);

        nodes = std::vector<model::CppAstNode>(result.begin(), result.end());

        break;
      }

      case ENUM_CONSTANTS:
      {
        node = queryCppAstNode(astNodeId_);

        EnumResult cppEnums = _db->query<model::CppEnum>(
          EnumQuery::mangledNameHash == node.mangledNameHash);
        model::CppEnum cppEnum = *cppEnums.begin();

        std::transform(
          cppEnum.enumConstants.begin(),
          cppEnum.enumConstants.end(),
          std::back_inserter(nodes),
          [this](const auto& enumConst) {
            return this->queryCppAstNode(
              std::to_string(enumConst.load()->astNodeId));
          });

        break;
      }
    }

    std::sort(nodes.begin(), nodes.end(), compareByValue);

    return_.reserve(nodes.size());
    std::transform(
      nodes.begin(), nodes.end(),
      std::back_inserter(return_),
      CreateAstNodeInfo(_db, getTags(nodes)));
  });
}

void CppServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  switch (node.symbolType)
  {
    case model::CppAstNode::SymbolType::Function:
      return_["Function call diagram"] = FUNCTION_CALL;
      break;

    case model::CppAstNode::SymbolType::Type:
      return_["Detailed class diagram"] = DETAILED_CLASS;
      return_["Class collaboration diagram"] = CLASS_COLLABORATION;
      break;

    default: // Just to suppress warning of uncovered enum constants.
      break;
  }
}

void CppServiceHandler::getReferencesInFile(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const core::FileId& fileId_,
  const std::vector<std::string>& tags_)
{
  // TODO
}

void CppServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::int32_t pageSize_,
  const std::int32_t pageNo_)
{
  // TODO
}

void CppServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  return_["Types"]     = TYPES;
  return_["Functions"] = FUNCTIONS;
}

void CppServiceHandler::getFileReferences(
  std::vector<AstNodeInfo>& return_,
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{
  std::map<model::CppAstNodeId, std::vector<std::string>> tags;
  std::vector<model::CppAstNode> nodes;

  _transaction([&, this](){
    switch (referenceId_)
    {
      case TYPES:
        nodes = queryCppAstNodesInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::Type &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);
        break;

      case FUNCTIONS:
        nodes = queryCppAstNodesInFile(fileId_,
           AstQuery::symbolType == model::CppAstNode::SymbolType::Function &&
          (AstQuery::astType == model::CppAstNode::AstType::Definition ||
           AstQuery::astType == model::CppAstNode::AstType::Declaration));
        break;
    }

    std::sort(nodes.begin(), nodes.end(), compareByValue);

    return_.reserve(nodes.size());
    std::transform(
      nodes.begin(), nodes.end(),
      std::back_inserter(return_),
      CreateAstNodeInfo(_db, getTags(nodes)));
  });
}

void CppServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileId& fileId)
{
  // TODO
}

void CppServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_)
{
  Diagram diagram(_db, _datadir, _config);
  util::Graph graph;

  switch (diagramId_)
  {
    case FUNCTION_CALL:
      diagram.getFunctionCallDiagram(graph, astNodeId_);
      break;

    case DETAILED_CLASS:
      diagram.getDetailedClassDiagram(graph, astNodeId_);
      break;

    case CLASS_COLLABORATION:
      diagram.getClassCollaborationDiagram(graph, astNodeId_);
      break;
  }

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);
}

void CppServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  Diagram diagram(_db, _datadir, _config);

  switch (diagramId_)
  {
    case FUNCTION_CALL:
      return_ = diagram.getFunctionCallLegend();
      break;

    case CLASS_COLLABORATION:
      return_ = diagram.getClassCollaborationLegend();
      break;
  }
}

void CppServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  // TODO
}

void CppServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_)
{
  // TODO
}

void CppServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  // TODO
}

bool CppServiceHandler::compareByPosition(
  const model::CppAstNode& lhs,
  const model::CppAstNode& rhs)
{
  return lhs.location.range.start < rhs.location.range.start;
}

bool CppServiceHandler::compareByValue(
  const model::CppAstNode& lhs,
  const model::CppAstNode& rhs)
{
  return lhs.astValue < rhs.astValue;
}

model::CppAstNode CppServiceHandler::queryCppAstNode(
  const core::AstNodeId& astNodeId_)
{
  return _transaction([&, this](){
    model::CppAstNode node;

    if (!_db->find(std::stoull(astNodeId_), node))
    {
      core::InvalidId ex;
      ex.__set_msg("Invalid CppAstNode ID");
      ex.__set_nodeid(astNodeId_);
      throw ex;
    }

    return node;
  });
}

std::vector<model::CppAstNode> CppServiceHandler::queryCppAstNodes(
  const core::AstNodeId& astNodeId_,
  const AstQuery& query_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  AstResult result = _db->query<model::CppAstNode>(
    AstQuery::mangledNameHash == node.mangledNameHash &&
    AstQuery::location.range.end.line != model::Position::npos &&
    query_);

  return std::vector<model::CppAstNode>(result.begin(), result.end());
}

std::vector<model::CppAstNode> CppServiceHandler::queryCppAstNodesInFile(
  const core::FileId& fileId_,
  const odb::query<model::CppAstNode>& query_)
{
  AstResult result = _db->query<model::CppAstNode>(
    AstQuery::location.file == std::stoull(fileId_) && query_);

  return std::vector<model::CppAstNode>(result.begin(), result.end());
}

std::vector<model::CppAstNode> CppServiceHandler::queryDefinitions(
  const core::AstNodeId& astNodeId_)
{
  return queryCppAstNodes(
    astNodeId_,
    AstQuery::astType == model::CppAstNode::AstType::Definition);
}

std::vector<model::CppAstNode> CppServiceHandler::queryCalls(
  const core::AstNodeId& astNodeId_)
{
  std::vector<model::CppAstNode> nodes = queryDefinitions(astNodeId_);

  if (nodes.empty())
    return nodes;

  model::CppAstNode node = nodes.front();

  const model::Position& start = node.location.range.start;
  const model::Position& end = node.location.range.end;

  AstResult result = _db->query<model::CppAstNode>(
    AstQuery::location.file == node.location.file.object_id() &&
    (AstQuery::astType == model::CppAstNode::AstType::Usage ||
     AstQuery::astType == model::CppAstNode::AstType::VirtualCall) &&
    AstQuery::symbolType == model::CppAstNode::SymbolType::Function &&
    // StartPos >= Pos
    ((AstQuery::location.range.start.line == start.line &&
      AstQuery::location.range.start.column >= start.column) ||
     AstQuery::location.range.start.line > start.line) &&
    // Pos > EndPos
    ((AstQuery::location.range.end.line == end.line &&
      AstQuery::location.range.end.column < end.column) ||
     AstQuery::location.range.end.line < end.line));

  nodes = std::vector<model::CppAstNode>(result.begin(), result.end());

  return nodes;
}

std::vector<model::CppAstNode> CppServiceHandler::queryOverrides(
  const core::AstNodeId& astNodeId_,
  bool reverse_)
{
  std::vector<model::CppAstNode> nodes;

  model::CppAstNode node = queryCppAstNode(astNodeId_);

  std::unordered_set<std::uint64_t> overrides
    = transitiveClosureOfRel(
        model::CppRelation::Kind::Override,
        node.mangledNameHash,
        reverse_);

  std::transform(
    overrides.begin(),
    overrides.end(),
    std::back_inserter(nodes),
    [this](std::uint64_t mnh){
      AstResult result
        = _db->query<model::CppAstNode>(AstQuery::mangledNameHash == mnh);

      return *result.begin();
    });

  return nodes;
}

std::unordered_set<std::uint64_t>
CppServiceHandler::transitiveClosureOfRel(
  model::CppRelation::Kind kind_,
  std::uint64_t to_,
  bool reverse_)
{
  std::unordered_set<std::uint64_t> ret;

  std::queue<std::uint64_t> q;
  q.push(to_);

  while (!q.empty())
  {
    std::uint64_t current = q.front();
    q.pop();

    RelResult result = _db->query<model::CppRelation>(
      (reverse_ ? RelQuery::lhs : RelQuery::rhs) == current &&
      RelQuery::kind == kind_);

    for (const model::CppRelation relation : result)
    {
      std::uint64_t otherSide = reverse_ ? relation.rhs : relation.lhs;

      if (ret.find(otherSide) == ret.end())
      {
        ret.insert(otherSide);
        q.push(otherSide);
      }
    }
  }

  return ret;
}

std::map<model::CppAstNodeId, std::vector<std::string>>
CppServiceHandler::getTags(const std::vector<model::CppAstNode>& nodes_)
{
  std::map<model::CppAstNodeId, std::vector<std::string>> tags;

  for (const model::CppAstNode& node : nodes_)
  {
    std::vector<cc::model::CppAstNode> defs
      = queryDefinitions(std::to_string(node.id));

    const model::CppAstNode& defNode = defs.empty() ? node : defs.front();

    switch (node.symbolType)
    {
      case model::CppAstNode::SymbolType::Function:
      {
        for (const model::CppMemberType& mem : _db->query<model::CppMemberType>(
          (MemTypeQuery::memberAstNode == defNode.id ||
           MemTypeQuery::memberAstNode == node.id) &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Method))
        {
          //--- Visibility Tag---//

          std::string visibility
            = cc::model::visibilityToString(mem.visibility);

          if (!visibility.empty())
            tags[node.id].push_back(visibility);
        }

        //--- Virtual Tag ---//

        FuncResult funcNodes = _db->query<cc::model::CppFunction>(
          FuncQuery::mangledNameHash == defNode.mangledNameHash);
        const model::CppFunction& funcNode = *funcNodes.begin();

        for (const model::Tag& tag : funcNode.tags)
          tags[node.id].push_back(model::tagToString(tag));

        break;
      }

      case model::CppAstNode::SymbolType::Variable:
      {
        for (const model::CppMemberType& mem : _db->query<model::CppMemberType>(
          (MemTypeQuery::memberAstNode == defNode.id ||
           MemTypeQuery::memberAstNode == node.id) &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Field))
        {
          //--- Visibility Tag---//

          std::string visibility = model::visibilityToString(mem.visibility);

          if (!visibility.empty())
            tags[node.id].push_back(visibility);
        }

        //--- Global Tag ---//

        VarResult varNodes = _db->query<cc::model::CppVariable>(
          VarQuery::mangledNameHash == defNode.mangledNameHash);
        const model::CppVariable& varNode = *varNodes.begin();

        for (const model::Tag& tag : varNode.tags)
          tags[node.id].push_back(model::tagToString(tag));

        break;
      }
    }
  }

  return tags;
}

}
}
}    
