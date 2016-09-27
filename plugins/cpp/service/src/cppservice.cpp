#include <algorithm>
#include <queue>

#include <boost/log/trivial.hpp>

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
    CreateAstNodeInfo(
      const std::map<cc::model::CppAstNodeId, std::vector<std::string>>& tags_ = {})
        : _tags(tags_)
    {
    }

    cc::service::language::AstNodeInfo operator()(
      const cc::model::CppAstNode& astNode_)
    {
      cc::service::language::AstNodeInfo ret;

      ret.__set_id(std::to_string(astNode_.id));
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
        ret.__set_srcText(cc::util::textRange(
          astNode_.location.file.load()->content.load()->content,
          astNode_.location.range.start.line,
          astNode_.location.range.start.column,
          astNode_.location.range.end.line,
          astNode_.location.range.end.column));
      }

      if(_tags.count(astNode_.id) != 0)
        ret.tags = _tags.at(astNode_.id);

      return ret;
    }

    const std::map<cc::model::CppAstNodeId, std::vector<std::string>>& _tags;
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
  const boost::program_options::variables_map& config_)
    : _db(db_), _transaction(db_), _config(config_)
{
}

static const std::vector<std::string> extension = {"CPP"};

void CppServiceHandler::getFileTypes(
  std::map<std::string, std::int32_t>& _return)
{
  for(const std::string& ext_ : extension)
    _return[ext_] = util::fnvHash(ext_);
}

void CppServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{
  return_ = CreateAstNodeInfo()(queryCppAstNode(astNodeId_));
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

    return_ = CreateAstNodeInfo()(min);
  });
}

void CppServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  _transaction([&, this](){
    model::CppAstNode node = queryCppAstNode(astNodeId_);

    std::vector<model::CppAstNode> defs = queryCppAstNodes(
      astNodeId_,
      AstQuery::astType == model::CppAstNode::AstType::Definition);

    if (defs.empty())
      return;

    switch (node.symbolType)
    {
      case model::CppAstNode::SymbolType::Variable:
      {
        model::CppVariable variable = _db->query_value<model::CppVariable>(
          VarQuery::mangledNameHash == defs.front().mangledNameHash);

        return_["Name"] = variable.name;
        return_["Qualified name"] = variable.qualifiedName;
        return_["Type"] = variable.qualifiedType;
        break;
      }

      case model::CppAstNode::SymbolType::Function:
      {
        model::CppFunction function = _db->query_value<model::CppFunction>(
          FuncQuery::mangledNameHash == defs.front().mangledNameHash);

        return_["Name"] = function.qualifiedName.substr(
          function.qualifiedName.find_last_of(':') + 1);
        return_["Qualified name"] = function.qualifiedName;
        return_["Signature"] = function.name;

        break;
      }

      case model::CppAstNode::SymbolType::Type:
      {
        model::CppType type = _db->query_value<model::CppType>(
          TypeQuery::mangledNameHash == defs.front().mangledNameHash);

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
        model::CppTypedef type = _db->query_value<model::CppTypedef>(
          TypedefQuery::astNodeId == node.id);

        return_["Name"] = type.name;
        return_["Qualified name"] = type.qualifiedName;

        break;
      }

      case model::CppAstNode::SymbolType::EnumConstant:
      {
        model::CppEnumConstant enumConst
          = _db->query_value<model::CppEnumConstant>(
              EnumConstQuery::astNodeId == node.id);

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

  return_["Definition"]                    = DEFINITION;
  return_["Declaration"]                   = DECLARATION;
  return_["Usage"]                         = USAGE;

  switch (node.symbolType)
  {
    case model::CppAstNode::SymbolType::Function:
      return_["Callee"]                    = CALLEE;
      return_["Caller"]                    = CALLER;
      return_["Virtual call"]              = VIRTUAL_CALL;
      return_["Function pointer call"]     = FUNC_PTR_CALL;
      return_["Parameters"]                = PARAMETER;
      return_["Local variables"]           = LOCAL_VAR;
      break;

    case model::CppAstNode::SymbolType::Variable:
      return_["Reads"]                     = READ;
      return_["Writes"]                    = WRITE;
      break;

    case model::CppAstNode::SymbolType::Type:
      return_["Aliases"]                   = ALIAS;
      return_["Inherits from"]             = INHERIT_FROM;
      return_["Inherited by"]              = INHERIT_BY;
      return_["Data member"]               = DATA_MEMBER;
      return_["Method"]                    = METHOD;
      return_["Friends"]                   = FRIEND;
      return_["Usage"]                     = USAGE;
      break;

    case model::CppAstNode::SymbolType::Typedef:
      return_["Underlying type"]           = UNDERLYING_TYPE;
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

      case CALLEE:
      {
        nodes = queryDefinitions(astNodeId_);

        if (nodes.empty())
          break;

        node = nodes.front();
        nodes.clear();

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
        std::sort(nodes.begin(), nodes.end(), compareByPosition);

        break;
      }

      case CALLER:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Usage);
        break;

      case VIRTUAL_CALL:
      {
        node = queryCppAstNode(astNodeId_);

        std::unordered_set<std::uint64_t> overriddens
          = reverseTransitiveClosureOfRel(
              model::CppRelation::Kind::Override,
              node.mangledNameHash);

        for (std::uint64_t mangledNameHash : overriddens)
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == mangledNameHash &&
            AstQuery::astType == model::CppAstNode::AstType::VirtualCall);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;
      }

      case FUNC_PTR_CALL:
      {
        node = queryCppAstNode(astNodeId_);

        std::unordered_set<std::uint64_t> fptrCallers
          = reverseTransitiveClosureOfRel(
              model::CppRelation::Kind::Assign,
              node.mangledNameHash);

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

        model::CppFunction function = _db->query_value<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);

        for (auto var : function.parameters)
          nodes.push_back(queryCppAstNode(
            std::to_string(var.load()->astNodeId.get())));

        break;
      }

      case LOCAL_VAR:
      {
        node = queryCppAstNode(astNodeId_);

        model::CppFunction function = _db->query_value<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);

        for (auto var : function.locals)
          nodes.push_back(queryCppAstNode(
            std::to_string(var.load()->astNodeId.get())));

        break;
      }

      case READ:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Read);

        std::sort(nodes.begin(), nodes.end(), compareByPosition);

        break;

      case WRITE:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Write);

        std::sort(nodes.begin(), nodes.end(), compareByPosition);

        break;

      case ALIAS:
      {
        node = queryCppAstNode(astNodeId_);

        std::unordered_set<std::uint64_t> aliasSet
          = reverseTransitiveClosureOfRel(
              model::CppRelation::Kind::Alias,
              node.mangledNameHash);

        for (std::uint64_t alias : aliasSet)
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == alias);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;
      }

      case INHERIT_FROM:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppInheritance& inh : _db->query<model::CppInheritance>(
          InhQuery::derived == node.mangledNameHash))  // TODO: Filter by tags
        {
          AstResult result = _db->query<model::CppAstNode>(
            AstQuery::mangledNameHash == inh.base &&
            AstQuery::astType == model::CppAstNode::AstType::Definition);
          nodes.insert(nodes.end(), result.begin(), result.end());
        }

        break;

      case INHERIT_BY:
        node = queryCppAstNode(astNodeId_);

        for (const model::CppInheritance& inh : _db->query<model::CppInheritance>(
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
          MemTypeQuery::kind == model::CppMemberType::Kind::Field)) // TODO: Filter by tags
        {
          model::CppAstNodePtr astNode = mem.memberAstNode.load();

          //--- Visibility Tag---//

          std::string visibility =
            mem.visibility == model::Visibility::Private   ? "private" :
            mem.visibility == model::Visibility::Public    ? "public" :
            mem.visibility == model::Visibility::Protected ? "protected" : "";

          if(visibility != "")
            tags[astNode->id].push_back(visibility);

          //--- Static Tag ---//

          if(mem.isStatic)
            tags[astNode->id].push_back("static");

          if(astNode->location.range.end.line != model::Position::npos)
            nodes.push_back(*astNode);
        }

        break;

      case METHOD:
      {
        node = queryCppAstNode(astNodeId_);

        for (const model::CppMemberType& mem : _db->query<model::CppMemberType>(
          MemTypeQuery::typeHash == node.mangledNameHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Method // TODO: Filter by tags
        ))
        {
          model::CppAstNodePtr astNode = mem.memberAstNode.load();

          //--- Visibility Tag---//

          std::string visibility =
            mem.visibility == model::Visibility::Private   ? "private" :
            mem.visibility == model::Visibility::Public    ? "public" :
            mem.visibility == model::Visibility::Protected ? "protected" : "";

          if(visibility != "")
            tags[astNode->id].push_back(visibility);

          //--- Static Tag ---//

          if(mem.isStatic)
            tags[astNode->id].push_back("static");

          //--- Virtual Tag ---//

          core::AstNodeId nodeId = std::to_string(astNode->id);
          std::vector<model::CppAstNode> defs = queryDefinitions(nodeId);
          if(!defs.empty())
          {
            model::CppFunctionPtr funcNode = _db->query_one<model::CppFunction>(
              FuncQuery::mangledNameHash == defs.front().mangledNameHash);
            if(funcNode && funcNode->isVirtual)
              tags[astNode->id].push_back("virtual");
          }

          if(astNode->location.range.end.line != model::Position::npos)
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

        model::CppTypedef type = _db->query_value<model::CppTypedef>(
          TypedefQuery::mangledNameHash == node.mangledNameHash);

        AstResult result = _db->query<model::CppAstNode>(
          AstQuery::mangledNameHash == type.typeHash &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);

        nodes = std::vector<model::CppAstNode>(result.begin(), result.end());

        break;
      }

      case ENUM_CONSTANTS:
      {
        node = queryCppAstNode(astNodeId_);

        model::CppEnum cppEnum = _db->query_value<model::CppEnum>(
          EnumQuery::mangledNameHash == node.mangledNameHash);

        std::transform(
          cppEnum.enumConstants.begin(),
          cppEnum.enumConstants.end(),
          std::back_inserter(nodes),
          [this](const auto& enumConst) {
            return this->queryCppAstNode(
              std::to_string(enumConst.load()->astNodeId.get()));
          });

        break;
      }
    }

    return_.reserve(nodes.size());
    std::transform(
      nodes.begin(), nodes.end(),
      std::back_inserter(return_),
      CreateAstNodeInfo(tags));
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
      return_["UML class overview diagram"] = CLASS_OVERVIEW;
      return_["Class collaboration diagram"] = CLASS_COLLABORATION;

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
  // TODO
}

void CppServiceHandler::getFileReferences(
  std::vector<core::FileInfo>& return_,
  const core::FileId& fileId,
  const std::int32_t referenceId_)
{
  // TODO
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
  // TODO
}

void CppServiceHandler::getDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_)
{
  // TODO
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
    AstQuery::location.range.end.line != -1 &&
    query_);

  return std::vector<model::CppAstNode>(result.begin(), result.end());
}

std::vector<model::CppAstNode> CppServiceHandler::queryDefinitions(
  const core::AstNodeId& astNodeId_)
{
  return queryCppAstNodes(
    astNodeId_,
    AstQuery::astType == model::CppAstNode::AstType::Definition);
}

std::unordered_set<std::uint64_t>
CppServiceHandler::reverseTransitiveClosureOfRel(
  model::CppRelation::Kind kind_,
  std::uint64_t to_)
{
  std::unordered_set<std::uint64_t> ret;

  std::queue<std::uint64_t> q;
  q.push(to_);

  while (!q.empty())
  {
    std::uint64_t current = q.front();
    q.pop();

    RelResult result = _db->query<model::CppRelation>(
      RelQuery::rhs == current && RelQuery::kind == kind_);

    for (const model::CppRelation relation : result)
      if (ret.find(relation.lhs) == ret.end())
      {
        ret.insert(relation.lhs);
        q.push(relation.lhs);
      }
  }

  return ret;
}

}
}
}    
