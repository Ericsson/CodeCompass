#include <algorithm>
#include <queue>

#include <util/util.h>
#include <util/logutil.h>

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
#include <model/cppmacroexpansion.h>
#include <model/cppmacroexpansion-odb.hxx>
#include <model/cppdoccomment.h>
#include <model/cppdoccomment-odb.hxx>

#include <service/cppservice.h>

#include "diagram.h"
#include "filediagram.h"

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
  typedef odb::query<cc::model::CppMacroExpansion> MacroExpansionQuery;
  typedef odb::result<cc::model::CppMacroExpansion> MacroExpansionResult;
  typedef odb::query<cc::model::File> FileQuery;
  typedef odb::result<cc::model::File> FileResult;
  typedef odb::query<cc::model::CppDocComment> DocCommentQuery;
  typedef odb::result<cc::model::CppDocComment> DocCommentResult;

  /**
   * This struct transforms a model::CppAstNode to an AstNodeInfo Thrift
   * object.
   */
  struct CreateAstNodeInfo
  {
    typedef std::map<cc::model::CppAstNodeId, std::vector<std::string>> TagMap;

    CreateAstNodeInfo(const TagMap& tags_ = {}) : _tags(tags_)
    {
    }

    /**
     * Returns the Thrift object for this C++ AST node.
     */
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
        ret.range.file = std::to_string(astNode_.location.file.object_id());

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
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
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
  return_ = _transaction([this, &astNodeId_](){
    return CreateAstNodeInfo()(queryCppAstNode(astNodeId_));
  });
}

void CppServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  return_ = _transaction([this, &astNodeId_](){
    model::CppAstNode astNode = queryCppAstNode(astNodeId_);

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

void CppServiceHandler::getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_)
{
  _transaction([&, this](){
    model::CppAstNode node = queryCppAstNode(astNodeId_);

    DocCommentResult docComment = _db->query<model::CppDocComment>(
      DocCommentQuery::mangledNameHash == node.mangledNameHash);

    if (!docComment.empty())
      return_ = "<div class=\"main-doc\">" + docComment.begin()->contentHTML
        + "</div>";

    switch (node.symbolType)
    {
      case model::CppAstNode::SymbolType::Type:
      {
        //--- Data members ---//

        std::vector<AstNodeInfo> methods;
        getReferences(methods, astNodeId_, METHOD, {});

        for (const AstNodeInfo& method : methods)
        {
          std::map<std::string, std::string> properties;
          getProperties(properties, method.id);

          return_ += "<div class=\"group\"><div class=\"signature\">";

          //--- Add tags ---/

          for (const std::string& tag : method.tags)
            if (tag == "public" || tag == "private" || tag == "protected")
              return_ += "<span class=\"icon-visibility icon-" + tag
                      +  "\"></span>";
            else
              return_ += "<span class=\"tag tag-" + tag +"\" title=\""
                      +  tag + "\">" + (char)std::toupper(tag[0]) + "</span>";

          auto signature = properties.find("Signature");
          return_
            += signature == properties.end()
            ?  method.astNodeValue
            :  signature->second;

          return_ += "</div>";

          //--- Query documentation of members ---//

          DocCommentResult doc = _db->query<model::CppDocComment>(
            DocCommentQuery::mangledNameHash == method.mangledNameHash);

          if (!doc.empty())
            return_ += doc.begin()->contentHTML;

          return_ += "</div>";
        }
      }
    }
  });
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

    return_ = _transaction([this, &min](){
      return CreateAstNodeInfo(getTags({min}))(min);
    });
  });
}

void CppServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  _transaction([&, this](){
    model::CppAstNode node = queryCppAstNode(astNodeId_);

    switch (node.symbolType)
    {
      case model::CppAstNode::SymbolType::Variable:
      {
        VarResult variables = _db->query<model::CppVariable>(
          VarQuery::mangledNameHash == node.mangledNameHash);
        model::CppVariable variable = *variables.begin();

        return_["Name"] = variable.name;
        return_["Qualified name"] = variable.qualifiedName;
        return_["Type"] = variable.qualifiedType;
        break;
      }

      case model::CppAstNode::SymbolType::Function:
      {
        FuncResult functions = _db->query<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);
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
          TypeQuery::mangledNameHash == node.mangledNameHash);
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
          TypedefQuery::mangledNameHash == node.mangledNameHash);
        model::CppTypedef type = *types.begin();

        return_["Name"] = type.name;
        return_["Qualified name"] = type.qualifiedName;

        break;
      }

      case model::CppAstNode::SymbolType::EnumConstant:
      {
        EnumConstResult enumConsts
          = _db->query<model::CppEnumConstant>(
              EnumConstQuery::mangledNameHash == node.mangledNameHash);
        model::CppEnumConstant enumConst = *enumConsts.begin();

        return_["Name"] = enumConst.name;
        return_["Qualified name"] = enumConst.qualifiedName;
        return_["Value"] = std::to_string(enumConst.value);
      }
    }
  });
}

std::int32_t CppServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  return _transaction([&, this]() -> std::int32_t {
    switch (referenceId_)
    {
      case DEFINITION:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Definition);

      case DECLARATION:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Declaration &&
          AstQuery::visibleInSourceCode == true);

      case USAGE:
        return queryCppAstNodeCount(astNodeId_);

      case THIS_CALLS:
        return queryCallsCount(astNodeId_);

      case CALLS_OF_THIS:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Usage);

      case CALLEE:
      {
        std::int32_t count = 0;

        std::set<std::uint64_t> defHashes;
        for (const model::CppAstNode& call : queryCalls(astNodeId_))
        {
          model::CppAstNode node = queryCppAstNode(std::to_string(call.id));
          defHashes.insert(node.mangledNameHash);
        }

        if (!defHashes.empty())
          count += _db->query_value<model::CppAstCount>(
            AstQuery::mangledNameHash.in_range(
              defHashes.begin(), defHashes.end()) &&
            AstQuery::astType == model::CppAstNode::AstType::Definition &&
            AstQuery::location.range.end.line != model::Position::npos).count;

        return count;
      }

      case CALLER:
      {
        std::vector<AstNodeInfo> references;
        getReferences(references, astNodeId_, CALLER, {});
        return references.size();
      }

      case VIRTUAL_CALL:
      {
        std::int32_t count = queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::VirtualCall);

        for (const model::CppAstNode& node : queryOverrides(astNodeId_, true))
          count += queryCppAstNodeCount(std::to_string(node.id),
            AstQuery::astType == model::CppAstNode::AstType::VirtualCall);

        return count;
      }

      case FUNC_PTR_CALL:
      {
        std::int32_t count = 0;

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
          count += result.size();
        }

        return count;
      }

      case PARAMETER:
        return _db->query_value<model::CppFunctionParamCount>(
          FuncQuery::astNodeId == node.id).count;

      case LOCAL_VAR:
        return _db->query_value<model::CppFunctionLocalCount>(
          FuncQuery::astNodeId == node.id).count;

      case RETURN_TYPE:
      {
        node = queryCppAstNode(astNodeId_);

        FuncResult functions = _db->query<cc::model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);

        const model::CppFunction& function = *functions.begin();

        return _db->query_value<model::CppTypeCount>(
          TypeQuery::mangledNameHash == function.typeHash).count;

        break;
      }

      case OVERRIDE:
        return queryOverridesCount(astNodeId_, true);

      case OVERRIDDEN_BY:
        return queryOverridesCount(astNodeId_, false);

      case READ:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Read);

      case WRITE:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::Write);

      case TYPE:
      {
        node = queryCppAstNode(astNodeId_);

        VarResult varNodes = _db->query<cc::model::CppVariable>(
          VarQuery::mangledNameHash == node.mangledNameHash);

        const model::CppVariable& variable = *varNodes.begin();

        return _db->query_value<model::CppTypeCount>(
          TypeQuery::mangledNameHash == variable.typeHash).count;

        break;
      }

      case ALIAS:
        return _db->query_value<model::CppTypedefCount>(
          TypedefQuery::typeHash == node.mangledNameHash).count;

      case INHERIT_FROM:
        return _db->query_value<model::CppInheritanceCount>(
          InhQuery::derived == node.mangledNameHash).count;

      case INHERIT_BY:
        return _db->query_value<model::CppInheritanceCount>(
          InhQuery::base == node.mangledNameHash).count;

      case DATA_MEMBER:
        return _db->query_value<model::CppMemberTypeCount>(
          MemTypeQuery::typeHash == node.mangledNameHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Field).count;

      case METHOD:
        return _db->query_value<model::CppMemberTypeCount>(
          MemTypeQuery::typeHash == node.mangledNameHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Method).count;

      case FRIEND:
        return _db->query_value<model::CppFriendshipCount>(
          FriendQuery::target == node.mangledNameHash).count;

      case UNDERLYING_TYPE:
        return _db->query_value<model::CppTypedefCount>(
          TypedefQuery::mangledNameHash == node.mangledNameHash).count;

      case ENUM_CONSTANTS:
        return _db->query_value<model::CppEnumConstantsCount>(
          EnumQuery::mangledNameHash == node.mangledNameHash).count;

      case EXPANSION:
        return _db->query_value<model::CppMacroExpansionCount>(
          MacroExpansionQuery::astNodeId == node.id).count;

      case UNDEFINITION:
        return queryCppAstNodeCount(astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::UnDefinition);

      default:
        return 0;
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
      return_["Return type"]           = RETURN_TYPE;
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

    case model::CppAstNode::SymbolType::Macro:
      return_["Expansions"]            = EXPANSION;
      return_["Undefinitions"]         = UNDEFINITION;
      break;
  }
}

void CppServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& /* tags_ */)
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

      case RETURN_TYPE:
      {
        node = queryCppAstNode(astNodeId_);

        FuncResult functions = _db->query<model::CppFunction>(
          FuncQuery::mangledNameHash == node.mangledNameHash);
        model::CppFunction function = *functions.begin();

        TypeResult result = _db->query<model::CppType>(
          TypeQuery::mangledNameHash == function.typeHash);

        for (const model::CppType& type : result)
        {
          std::vector<model::CppAstNode> defs =
            queryDefinitions(std::to_string(type.astNodeId));
          nodes.insert(nodes.end(), defs.begin(), defs.end());
        }

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

      case EXPANSION:
      {
        node = queryCppAstNode(astNodeId_);

        MacroExpansionResult mExpansions = _db->query<model::CppMacroExpansion>(
          MacroExpansionQuery::astNodeId == node.id);

        for (const auto& expansion : mExpansions)
        {
          node = queryCppAstNode(std::to_string(expansion.astNodeId));
          node.astValue = expansion.expansion;
          nodes.push_back(node);
        }

        break;
      }

      case UNDEFINITION:
        nodes = queryCppAstNodes(
          astNodeId_,
          AstQuery::astType == model::CppAstNode::AstType::UnDefinition);
        break;
    }

    std::sort(nodes.begin(), nodes.end(), compareByValue);

    return_.reserve(nodes.size());
    _transaction([this, &return_, &nodes](){
      std::transform(
        nodes.begin(), nodes.end(),
        std::back_inserter(return_),
        CreateAstNodeInfo(getTags(nodes)));
    });
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
  std::vector<AstNodeInfo>& /* return_ */,
  const core::AstNodeId& /* astNodeId_ */,
  const std::int32_t /* referenceId_ */,
  const core::FileId& /* fileId_ */,
  const std::vector<std::string>& /* tags_ */)
{
  // TODO
}

void CppServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& /* return_ */,
  const core::AstNodeId& /* astNodeId_ */,
  const std::int32_t /* referenceId_ */,
  const std::int32_t /* pageSize_ */,
  const std::int32_t /* pageNo_ */)
{
  // TODO
}

void CppServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& /* fileId_ */)
{
  return_["Types"]     = TYPES;
  return_["Functions"] = FUNCTIONS;
  return_["Includes"]  = INCLUDES;
  return_["Macros"]    = MACROS;
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

      case INCLUDES:
        nodes = queryCppAstNodesInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::File);
        break;

      case MACROS:
        nodes = queryCppAstNodesInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::Macro &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);
        break;
    }

    std::sort(nodes.begin(), nodes.end(), compareByValue);

    return_.reserve(nodes.size());
    _transaction([this, &return_, &nodes](){
      std::transform(
        nodes.begin(), nodes.end(),
        std::back_inserter(return_),
        CreateAstNodeInfo(getTags(nodes)));
    });
  });
}

std::int32_t CppServiceHandler::getFileReferenceCount(
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{
  return _transaction([&, this]() -> std::int32_t {
    switch (referenceId_)
    {
      case TYPES:
        return queryCppAstNodeCountInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::Type &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);
        break;

      case FUNCTIONS:
        return queryCppAstNodeCountInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::Function &&
          (AstQuery::astType == model::CppAstNode::AstType::Definition ||
           AstQuery::astType == model::CppAstNode::AstType::Declaration));
        break;

      case INCLUDES:
        return queryCppAstNodeCountInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::File);
        break;

      case MACROS:
        return queryCppAstNodeCountInFile(fileId_,
          AstQuery::symbolType == model::CppAstNode::SymbolType::Macro &&
          AstQuery::astType == model::CppAstNode::AstType::Definition);
        break;

      default:
        return 0;
    }
  });
}

void CppServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& /* return_ */,
  const core::FileId& /* fileId_ */)
{
  // TODO
}

void CppServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_)
{
  Diagram diagram(_db, _datadir, _context);
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
  Diagram diagram(_db, _datadir, _context);

  switch (diagramId_)
  {
    case FUNCTION_CALL:
      return_ = diagram.getFunctionCallLegend();
      break;

    case CLASS_COLLABORATION:
      return_ = diagram.getClassCollaborationLegend();
      break;

    case DETAILED_CLASS:
      return_ = diagram.getDetailedClassLegend();
      break;
  }
}

void CppServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == stoull(fileId_));
  });

  if (file->type == model::File::DIRECTORY_TYPE)
  {
    return_["Internal architecture of this module"] = SUBSYSTEM_DEPENDENCY;
    return_["This module depends on"]               = EXTERNAL_DEPENDENCY;
    return_["Users of this module"]                 = EXTERNAL_USERS;
  }
  else
  {
    return_["Include Dependency"]                   = INCLUDE_DEPENDENCY;
    return_["Component Users"]                      = COMPONENT_USERS;
    return_["Interface Diagram"]                    = INTERFACE;
  }
}

void CppServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_)
{
  FileDiagram diagram(_db, _datadir, _context);
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  switch (diagramId_)
  {
    case COMPONENT_USERS:
      diagram.getComponentUsersDiagram(graph, fileId_);
      break;

    case EXTERNAL_DEPENDENCY:
      diagram.getExternalDependencyDiagram(graph, fileId_);
      break;

    case EXTERNAL_USERS:
      diagram.getExternalUsersDiagram(graph, fileId_);
      break;

    case INCLUDE_DEPENDENCY:
      diagram.getIncludeDependencyDiagram(graph, fileId_);
      break;

    case INTERFACE:
      diagram.getInterfaceDiagram(graph, fileId_);
      break;

    case SUBSYSTEM_DEPENDENCY:
      diagram.getSubsystemDependencyDiagram(graph, fileId_);
      break;
  }

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);
}

void CppServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  FileDiagram diagram(_db, _datadir, _context);

  switch (diagramId_)
  {
    case COMPONENT_USERS:
      return_ = diagram.getComponentUsersDiagramLegend();
        break;

      case EXTERNAL_DEPENDENCY:
        return_ = diagram.getExternalDependencyDiagramLegend();
        break;

      case EXTERNAL_USERS:
        return_ = diagram.getExternalUsersDiagramLegend();
        break;

      case INCLUDE_DEPENDENCY:
        return_ = diagram.getIncludeDependencyDiagramLegend();
        break;

      case INTERFACE:
        return_ = diagram.getInterfaceDiagramLegend();
        break;

      case SUBSYSTEM_DEPENDENCY:
        return_ = diagram.getSubsystemDependencyDiagramLegend();
        break;
  }
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

std::uint32_t CppServiceHandler::queryCppAstNodeCountInFile(
  const core::FileId& fileId_,
  const odb::query<model::CppAstNode>& query_)
{
  return _db->query_value<model::CppAstCount>(
    AstQuery::location.file == std::stoull(fileId_) && query_).count;
}

std::vector<model::CppAstNode> CppServiceHandler::queryDefinitions(
  const core::AstNodeId& astNodeId_)
{
  return queryCppAstNodes(
    astNodeId_,
    AstQuery::astType == model::CppAstNode::AstType::Definition);
}

odb::query<model::CppAstNode> CppServiceHandler::astCallsQuery(
  const model::CppAstNode& astNode_)
{
  const model::Position& start = astNode_.location.range.start;
  const model::Position& end = astNode_.location.range.end;

  return (AstQuery::location.file == astNode_.location.file.object_id() &&
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
}

std::vector<model::CppAstNode> CppServiceHandler::queryCalls(
  const core::AstNodeId& astNodeId_)
{
  std::vector<model::CppAstNode> nodes = queryDefinitions(astNodeId_);

  if (nodes.empty())
    return nodes;

  model::CppAstNode node = nodes.front();
  AstResult result = _db->query<model::CppAstNode>(astCallsQuery(node));

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

std::size_t CppServiceHandler::queryCppAstNodeCount(
  const core::AstNodeId& astNodeId_,
  const AstQuery& query_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  model::CppAstCount q = _db->query_value<model::CppAstCount>(
    AstQuery::mangledNameHash == node.mangledNameHash &&
    AstQuery::location.range.end.line != model::Position::npos &&
    query_);

  return q.count;
}

std::size_t CppServiceHandler::queryOverridesCount(
  const core::AstNodeId& astNodeId_,
  bool reverse_)
{
  model::CppAstNode node = queryCppAstNode(astNodeId_);

  return transitiveClosureOfRel( model::CppRelation::Kind::Override,
    node.mangledNameHash, reverse_).size();
}

std::size_t CppServiceHandler::queryCallsCount(
  const core::AstNodeId& astNodeId_)
{
  std::vector<model::CppAstNode> nodes = queryDefinitions(astNodeId_);

  if (nodes.empty())
    return std::size_t(0);

  model::CppAstNode node = nodes.front();

  return _db->query_value<model::CppAstCount>(astCallsQuery(node)).count;
}

} // language
} // service
} // cc
