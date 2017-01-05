// $Id$
// Created by Aron Barath, 2013

#include "javaservicehelper/symbolhandler.h"
#include "util/streamlog.h"

// COPY FROM ../../cppservicehelper/src/cppservicehelper.cpp r4762

#include "model/java/javaastnode.h"
#include "model/java/javaastnode-odb.hxx"
#include "model/java/javatype.h"
#include "model/java/javatype-odb.hxx"
#include "model/java/javatype_interfaces.h"
#include "model/java/javatype_interfaces-odb.hxx"
#include "model/java/javatype_typeparams.h"
#include "model/java/javatype_typeparams-odb.hxx"
#include "model/java/javafunction.h"
#include "model/java/javafunction-odb.hxx"
#include "model/java/javafunction_typeparams.h"
#include "model/java/javafunction_typeparams-odb.hxx"
#include "model/java/javavariable.h"
#include "model/java/javavariable-odb.hxx"
#include "model/java/javaenumconstant.h"
#include "model/java/javaenumconstant-odb.hxx"
#include "model/java/javaimport.h"
#include "model/java/javaimport-odb.hxx"
#include "model/java/javamodifiers.h"

#include "utils.h"

namespace cc
{
namespace service
{
namespace language 
{
  
namespace
{ // unnamed namespace

auto sortByPosition = [](const model::JavaAstNode& lhs,
                         const model::JavaAstNode& rhs)
{
  return
    model::Position(lhs.loc_start_line, lhs.loc_start_col)
    <
    model::Position(rhs.loc_start_line, rhs.loc_start_col);
};

} // unnamed namespace

model::JavaAstNodePtr SymbolHandler::loadAstNodePtr(
  const model::JavaType & javaType,
  bool genericlevel)
{
  model::JavaAstNodePtr node = javaType.astNodePtr.load();

  if(genericlevel && javaType.genericImpl.load())
  {
    node = javaType.genericImpl.load()->astNodePtr.load();
  }

  return node;
}

model::JavaAstNodePtr SymbolHandler::loadAstNodePtr(
  const model::JavaFunction & javaFunction,
  bool genericlevel)
{
  model::JavaAstNodePtr node = javaFunction.astNodePtr.load();

  if(genericlevel && javaFunction.genericImpl.load())
  {
    node = javaFunction.genericImpl.load()->astNodePtr.load();
  }

  return node;
}

InfoNode SymbolHandler::makeInfoNode(
  const std::vector<std::string> & category_,
  const std::string & label_,
  const std::string & value_,
  const AstNodeInfo & astValue_)
{
  InfoNode infoNode;
  infoNode.category = category_;
  infoNode.label = label_;
  infoNode.value = value_;
  infoNode.astValue = astValue_;
  return infoNode;
}

InfoNode SymbolHandler::makeInfoNodeEx(std::vector<std::string> category_, std::string label_, model::JavaAstNodePtr node)
{
  if(node)
  {
    return makeInfoNode(category_, label_, getFileloc(*node), createAstNodeInfo(*node));
  }

  return makeInfoNode(category_, label_, "(external)");
}

InfoNode SymbolHandler::makeInfoNodeFromType(
  const std::vector<std::string> & category_,
  const std::string & label_,
  const model::JavaType & type_,
  model::JavaAstNodePtr node)
{
  if(node)
  {
    return makeInfoNode(category_, label_, type_.qualifiedName,
      createAstNodeInfo(*node));
  }

  return makeInfoNode(category_, label_, type_.qualifiedName);
}

InfoNode SymbolHandler::makeInfoNodeFromCall(
  const std::vector<std::string> & category_,
  const model::JavaFunction & func_,
  const model::JavaAstNode & call_)
{
  return makeInfoNode(category_,
    std::to_string(call_.loc_start_line) + ":" +
    std::to_string(call_.loc_start_col),
    textRange(call_.file.load()->content.load()->content,
      call_.loc_start_line, call_.loc_start_col,
      call_.loc_end_line, call_.loc_end_col),
    createAstNodeInfo(call_));
}

InfoNode SymbolHandler::makeInfoQueryNode(
  std::vector<std::string> category,
  int queryId,
  std::vector<std::string> filters)
{
  InfoQuery query;
  query.queryId = queryId;
  query.filters = std::move(filters);

  InfoNode ret;
  ret.category = std::move(category);
  ret.query = query;

  return ret;
}

// TODO: copy/paste from C++ SymbolHandler
std::vector<InfoNode> SymbolHandler::makeHitCountQueryNodes(
  std::shared_ptr<odb::database> db,
  odb::result<model::AstCountGroupByFilesJava> hitcounts,
  int queryId,
  std::vector<std::string> filters)
{
  typedef std::pair<model::File, unsigned> HitCount;
  std::vector<HitCount> hitcountsVec;
  hitcountsVec.reserve(128);

  for (const auto& hitcount : hitcounts)
  {
    model::File file;
    db->load<model::File>(hitcount.file, file);

    hitcountsVec.push_back( { std::move(file), hitcount.count });
  }

  std::sort(hitcountsVec.begin(), hitcountsVec.end(),
    [](const HitCount& lhs, const HitCount& rhs){
      return lhs.first.filename < rhs.first.filename;
  });

  std::vector<InfoNode> ret;
  ret.reserve(hitcountsVec.size());

  for (unsigned i = 0; i < hitcountsVec.size(); ++i)
  {
    const auto& hitcount = hitcountsVec[i];

    std::string cat = hitcount.first.filename + " ("
      + std::to_string(hitcount.second) + ')';

    if ((i > 0 && hitcount.first.filename == hitcountsVec[i-1].first.filename) ||
        (i < (hitcountsVec.size() - 1) &&
         hitcount.first.filename == hitcountsVec[i+1].first.filename))
    {
      cat += " in " + hitcount.first.path;
    }

    filters.push_back(std::to_string(hitcount.first.id));

    ret.push_back(makeInfoQueryNode( { cat }, queryId, filters));

    filters.pop_back();
  }

  return ret;
}

std::vector<InfoNode> SymbolHandler::getUsageOf(
  unsigned long long mangledNameHash_,
  model::AstType mode_,
  int subquery_,
  std::vector<std::string> filters_)
{
  auto resultSet = db->query<model::AstCountGroupByFilesJava>(
    JavaOdbQuery::astMangledNameHash(mangledNameHash_) &&
    JavaOdbQuery::astAstType(mode_));

  return makeHitCountQueryNodes(db, resultSet, subquery_, filters_);
}

std::vector<InfoNode> SymbolHandler::getUsageOfInFile(
  unsigned long long mangledNameHash,
  model::AstType mode,
  model::FileId fileId)
{
  return getUsageOfInFile(
    mangledNameHash,
    JavaOdbQuery::astAstType(mode),
    fileId);
}

std::vector<InfoNode> SymbolHandler::getUsageOfInFile(
  unsigned long long mangledNameHash,
  JavaOdbQuery::AstQuery query_,
  model::FileId fileId)
{
  return getUsageOfInFile(
    JavaOdbQuery::astMangledNameHash(mangledNameHash) &&
    query_ &&
    JavaOdbQuery::astFileId(fileId));
}

std::vector<InfoNode> SymbolHandler::getUsageOfInFile(JavaOdbQuery::AstQuery query_)
{
  std::vector<InfoNode> ret;

  for(auto ref : query.sortQuery<model::JavaAstNode>(query_, sortByPosition))
  {
    AstNodeInfo refInfo = createAstNodeInfo(ref);

    std::string filename = cc::service::language::baseName(ref.file.load()->path);
    std::string line = std::to_string(ref.loc_start_line);
    std::string column = std::to_string(ref.loc_start_col);

    ret.push_back(
      makeInfoNode( {}, line + ':' + column,
        refInfo.astNodeSrcText, refInfo));
  }

  return ret;
}

std::string SymbolHandler::getFunctionSignature(
  const model::JavaFunction & func_,
  bool withRetType_)
{
  if(withRetType_ && func_.returnType.load())
  {
    return func_.returnType->qualifiedName + " " + func_.mangledName;
  }

  return func_.mangledName;
}

std::vector<InfoNode> SymbolHandler::getAnnotationsFor(const model::JavaAstNode & astNode)
{
  std::vector<InfoNode> ret;

  typedef odb::query<model::JavaAnnotation> QAnn;

  for(model::JavaAnnotation & ann : db->query<model::JavaAnnotation>(QAnn::astNodePtr == astNode.id))
  {
    // TODO
//    if(ann.annotationType.load())
//    {
//      ret.push_back(makeInfoNodeEx({}, ann.name, ann.annotationType.load()->astNodePtr.load()));
//    }
//    else
    {
      ret.push_back(makeInfoNodeEx({}, ann.name, {}));
    }
  }

  return ret;
}

class TypeHandler : public SymbolHandler
{
  enum class SubQuery
  {
    interfaces = 1,
    methods,
    members,
    inhMethods,
    inhMembers,
    inheritedBy,
    inheritsFrom,
    annotations,
    enumconsts,
    usageLocal,
    usageField,
    usageParam,
    usageRetType,
    usageInFile,
    genericInst,
    typeArgs,
  };
public:
  TypeHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  struct empty_query_warning_exception { empty_query_warning_exception() { } };

  std::string getInfoBoxText(const model::JavaAstNode & astNode)
  {
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

    if(javaType.genericImpl.load() && javaType.genericImpl.load()->astNodePtr.load())
    {
      return SymbolHandler::getInfoBoxText(*javaType.genericImpl.load()->astNodePtr.load());
    }

    return SymbolHandler::getInfoBoxText(astNode);
  }

  void addGenericInstances(const model::JavaType & type,
    std::vector<model::JavaType::pktype> & instance_ids)
  {
    if(type.isGeneric)
    {
      typedef odb::query<model::JavaType> QType;

      for(const model::JavaType & inst : db->query<model::JavaType>(
        QType::genericImpl==type.mangledNameHash))
      {
        instance_ids.push_back(inst.mangledNameHash);
      }
    }
  }

  static int extractVisibility(int modifiers)
  {
    return modifiers &
      (model::JavaModifiers::Public    |
       model::JavaModifiers::Protected |
       model::JavaModifiers::Private);
  }

  bool getSuperClassInfoNode(const model::JavaType & javaType, InfoNode & infoNode)
  {
    if(javaType.superClass.load())
    {
      model::JavaAstNodePtr node = loadAstNodePtr(*javaType.superClass, false);

      if(node)
      {
        infoNode = makeInfoNode({}, javaType.superClass->name,
          getFileloc(*node), createAstNodeInfo(*node));
      }
      else
      {
        infoNode = makeInfoNode({}, javaType.superClass->name, "(external)");
      }

      return true;
    }

    return false;
  }

  std::vector<InfoNode> getInfoTree(const model::JavaAstNode& astNode) override
  {
    SLog(util::DEBUG) << "Trying to find JavaType with astnode id:" << astNode.id;

    try
    {
      std::vector<InfoNode> ret;

      model::JavaType javaType = query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

      // short name with signature
      ret.push_back(makeInfoNode({}, "Name", javaType.name));

      // qualified name
      ret.push_back(
        makeInfoNode({}, "Qualified Name", javaType.qualifiedName));

      model::JavaAstNodePtr realDefNode = loadAstNodePtr(javaType);

      // definition
      if(realDefNode && realDefNode.load()->file.load())
      {
        ret.push_back(
          makeInfoNode({}, "Defined", getFileloc(*realDefNode),
            createAstNodeInfo(*realDefNode)));
      }

      // superclass
      InfoNode superclassInfoNode;
      if(getSuperClassInfoNode(javaType, superclassInfoNode))
      {
        superclassInfoNode.value = superclassInfoNode.label;
        superclassInfoNode.label = "Super Class";
        ret.push_back(superclassInfoNode);
      }

      ret.push_back(makeInfoQueryNode({"Interfaces"}, int(SubQuery::interfaces)));
      ret.push_back(makeInfoQueryNode({"Inherits From"}, int(SubQuery::inheritsFrom)));
      ret.push_back(makeInfoQueryNode({"Inherited By"}, int(SubQuery::inheritedBy)));
      ret.push_back(makeInfoQueryNode({"Annotations"}, int(SubQuery::annotations)));
      ret.push_back(makeInfoQueryNode({"Methods"}, int(SubQuery::methods)));
      ret.push_back(makeInfoQueryNode({"Members"}, int(SubQuery::members)));
      ret.push_back(makeInfoQueryNode({"Usage", "Local"}, int(SubQuery::usageLocal),
        {std::to_string(astNode.mangledNameHash)}));
      ret.push_back(makeInfoQueryNode({"Usage", "Field"}, int(SubQuery::usageField),
        {std::to_string(astNode.mangledNameHash)}));
      ret.push_back(makeInfoQueryNode({"Usage", "Parameter"}, int(SubQuery::usageParam),
        {std::to_string(astNode.mangledNameHash)}));
      ret.push_back(makeInfoQueryNode({"Usage", "Return type"}, int(SubQuery::usageRetType),
        {std::to_string(astNode.mangledNameHash)}));

      if(javaType.isGeneric || javaType.genericImpl.load())
      {
        ret.push_back(makeInfoQueryNode({"Generic Instances"}, int(SubQuery::genericInst)));
      }

      if(javaType.genericImpl.load())
      {
        ret.push_back(makeInfoQueryNode({"Type Arguments"}, int(SubQuery::typeArgs)));
      }

      if(javaType.isEnum)
      {
        ret.push_back(makeInfoQueryNode({"Enum Constants"}, (int)SubQuery::enumconsts));
      }

      return ret;
    }
    catch(JavaOdbQuery::NoDefintionNorDeclaration & ex)
    {
      // valid for unknown types
      return {};
    }
  }

  std::vector<InfoNode> getInterfaces(const model::JavaAstNode& astNode)
  {
    auto javaType = query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

    std::vector<InfoNode> ret;

    typedef odb::query<model::JavaType_Interfaces> QTypeIf;

    for(model::JavaType_Interfaces & mapping : db->query<model::JavaType_Interfaces>(QTypeIf::type == javaType.mangledNameHash))
    {
      try
      {
        model::JavaAstNodePtr node = loadAstNodePtr(*mapping.iface.load());

        ret.push_back(makeInfoNodeEx({}, mapping.iface->name, node));
      }
      catch(const std::exception& ex)
      {
        SLog(util::ERROR)<< "Exception caught: " << ex.what();
      }
    }

    return ret;
  }

  std::vector<InfoNode> getInheritsFrom(const model::JavaAstNode& astNode)
  {
    std::vector<InfoNode> ret;

    for(model::JavaType & inh : getInheritsFromTypes(db, query, astNode))
    {
      ret.push_back(makeInfoNodeEx({}, inh.name, loadAstNodePtr(inh)));
    }

    return ret;
  }

  std::vector<InfoNode> getInheritedBy(const model::JavaAstNode& astNode)
  {
    std::vector<InfoNode> ret;

    for(model::JavaType & type : getInheritsByTypes(db, query, astNode, true))
    {
      ret.push_back(makeInfoNodeEx({}, type.name, loadAstNodePtr(type)));
    }

    return ret;
  }

  std::vector<InfoNode> getMethods(const model::JavaAstNode& astNode)
  {
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

    std::vector<InfoNode> ret;
    ret.reserve(javaType.functions.size());

    std::vector<InfoNode> pubMethods;
    std::vector<InfoNode> protMethods;
    std::vector<InfoNode> privMethods;
    std::vector<InfoNode> pkgMethods;

    for(const auto& func : javaType.functions)
    {
      try
      {
        auto memFunInfo = createAstNodeInfo(*func.load()->astNodePtr.load());

        std::string funcName = func.load()->name;

        std::string returnType(" ");
        if(func.load()->returnType.load())
        {
          returnType = func.load()->returnType.load()->name;
        }

        switch(extractVisibility(func.load()->modifiers))
        {
        case model::JavaModifiers::Public:
          pubMethods.push_back(
            makeInfoNode({"Public"}, func.load()->signature,
              returnType, memFunInfo));
          break;

        case model::JavaModifiers::Protected:
          protMethods.push_back(
            makeInfoNode({"Protected"}, func.load()->signature,
              returnType, memFunInfo));
          break;

        case model::JavaModifiers::Private:
          privMethods.push_back(
            makeInfoNode({"Private"}, func.load()->signature,
              returnType, memFunInfo));
          break;

        default:
          pkgMethods.push_back(
            makeInfoNode({"Package"}, func.load()->signature,
              returnType, memFunInfo));
          break;
        }
      }
      catch(const std::exception& ex)
      {
        SLog(util::ERROR)<< "Exception caught: " << ex.what();
      }
    }

    std::move(pubMethods.begin(), pubMethods.end(), std::back_inserter(ret));
    std::move(protMethods.begin(), protMethods.end(), std::back_inserter(ret));
    std::move(privMethods.begin(), privMethods.end(), std::back_inserter(ret));
    std::move(pkgMethods.begin(), pkgMethods.end(), std::back_inserter(ret));

    return ret;
  }

  std::vector<InfoNode> getMembers(const model::JavaAstNode& astNode)
  {
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);
    std::vector<InfoNode> ret;
    ret.reserve(javaType.fields.size());

    std::vector<InfoNode> pubMembers;
    std::vector<InfoNode> protMembers;
    std::vector<InfoNode> privMembers;
    std::vector<InfoNode> pkgMembers;

    for(const auto& member : javaType.fields)
    {
      try
      {
        AstNodeInfo memberInfo = createAstNodeInfo(*member.load()->astNodePtr.load());

        std::string typeStr;

        if(member->fieldType.load())
        {
//          typeStr = query.queryEntityByHash<model::JavaType>(member->fieldType->mangledNameHash).name;
          typeStr = member->fieldType->name;
        }

        switch(extractVisibility(member->modifiers))
        {
        case model::JavaModifiers::Public:
          pubMembers.push_back(
            makeInfoNode({"Public"}, member->name, typeStr, memberInfo));
          break;

        case model::JavaModifiers::Protected:
          protMembers.push_back(
            makeInfoNode({"Protected"}, member->name, typeStr, memberInfo));
          break;

        case model::JavaModifiers::Private:
          privMembers.push_back(
            makeInfoNode({"Private"}, member->name, typeStr, memberInfo));
          break;

        default:
          pkgMembers.push_back(
            makeInfoNode({"Package"}, member->name, typeStr, memberInfo));
          break;
        }
      }
      catch(const std::exception& ex)
      {
        SLog(util::ERROR)<< "Exception caught: " << ex.what();
      }
    }

    std::move(pubMembers.begin(), pubMembers.end(), std::back_inserter(ret));
    std::move(protMembers.begin(), protMembers.end(), std::back_inserter(ret));
    std::move(privMembers.begin(), privMembers.end(), std::back_inserter(ret));
    std::move(pkgMembers.begin(), pkgMembers.end(), std::back_inserter(ret));

    return ret;
  }

  JavaOdbQuery::AstQuery getUsageQuery(model::JavaType & type, SubQuery subQuery, std::vector<model::JavaAstNode::pktype> & helper)
  {
    helper.clear();

    typedef odb::query<model::JavaVariable> QVar;
    typedef odb::query<model::JavaMember> QMember;
    typedef odb::query<model::JavaFunction> QFunc;

    std::vector<model::JavaType::pktype> type_ids;
    type_ids.push_back(type.mangledNameHash);
    addGenericInstances(type, type_ids);

    switch(subQuery)
    {
    case SubQuery::usageLocal:
      {
        for(const model::JavaVariable & var : db->query<model::JavaVariable>(
          QVar::type.in_range(type_ids.begin(), type_ids.end()) &&
          QVar::paramInFunc.is_null() &&
          QVar::localInFunc.is_not_null()
        ))
        {
          helper.push_back(var.astNodePtr.load()->id);
        }
      }
      break;

    case SubQuery::usageField:
      {
        for(const model::JavaMember & mem : db->query<model::JavaMember>(
          (
            QMember::fieldType.in_range(type_ids.begin(), type_ids.end())
          )
        ))
        {
          helper.push_back(mem.astNodePtr.load()->id);
        }
      }
      break;

    case SubQuery::usageParam:
      {
        for(const model::JavaVariable & var : db->query<model::JavaVariable>(
          QVar::type.in_range(type_ids.begin(), type_ids.end()) &&
          QVar::paramInFunc.is_not_null() &&
          QVar::localInFunc.is_null()
        ))
        {
          helper.push_back(var.astNodePtr.load()->id);
        }
      }
      break;

    case SubQuery::usageRetType:
      {
        for(const model::JavaFunction & func : db->query<model::JavaFunction>(
          QFunc::returnType.is_not_null() &&
          QFunc::returnType.in_range(type_ids.begin(), type_ids.end())
        ))
        {
          helper.push_back(func.astNodePtr.load()->id);
        }
      }
      break;

    default:
      throw std::runtime_error("Invalid SubQuery in JavaTypeHandler::getQuery");
    }

   if(helper.empty())
    {
      throw empty_query_warning_exception();
    }

    return {JavaOdbQuery::AstQuery::id.in_range(helper.begin(), helper.end())};
  }

  std::vector<InfoNode> getSubInfoTree(const model::JavaAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    SubQuery subQuery = static_cast<SubQuery>(infoQuery.queryId);

    switch(subQuery)
    {
    case SubQuery::interfaces:
      {
        return getInterfaces(astNode);
      }
      break;

    case SubQuery::methods:
      {
        std::vector<InfoNode> ret = getMethods(astNode);
        ret.push_back(makeInfoQueryNode({"Inherited"}, (int)SubQuery::inhMethods));
        return ret;
      }
      break;

    case SubQuery::members:
      {
        std::vector<InfoNode> ret = getMembers(astNode);
        ret.push_back(makeInfoQueryNode({"Inherited"}, (int)SubQuery::inhMembers));
        return ret;
      }
      break;

    case SubQuery::inhMembers:
      {
        std::vector<InfoNode> ret;

        for(model::JavaType & type : getInheritsFromTypes(db, query, astNode))
        {
          if(model::JavaAstNodePtr node = loadAstNodePtr(type))
          {
            std::vector<InfoNode> members = getMembers(*node);
            std::move(members.begin(), members.end(), std::back_inserter(ret));
          }
        }

        return ret;
      }
      break;

    case SubQuery::inhMethods:
      {
        std::vector<InfoNode> ret;

        for(model::JavaType & type : getInheritsFromTypes(db, query, astNode))
        {
          if(model::JavaAstNodePtr node = loadAstNodePtr(type))
          {
            std::vector<InfoNode> methods = getMethods(*node);
            std::move(methods.begin(), methods.end(), std::back_inserter(ret));
          }
        }

        return ret;
      }
      break;

    case SubQuery::inheritsFrom:
      {
        return getInheritsFrom(astNode);
      }
      break;

    case SubQuery::inheritedBy:
      {
        return getInheritedBy(astNode);
      }
      break;

    case SubQuery::enumconsts:
      {
        model::JavaType javaType =
         query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

        std::vector<InfoNode> ret;

        for(odb::lazy_shared_ptr<model::JavaEnumConstant> ec : javaType.enumConstants)
        {
          ret.push_back(makeInfoNode({}, ec.load()->name, javaType.name,
            createAstNodeInfo(*ec.load()->astNodePtr.load())));
        }

        return ret;
      }
      break;

    case SubQuery::usageLocal:
    case SubQuery::usageField:
    case SubQuery::usageParam:
    case SubQuery::usageRetType:
      {
        if(infoQuery.filters.empty())
        {
          throw std::runtime_error("Empty InfoQuery::filters in TypeHandler::usage");
        }

        std::vector<model::JavaAstNode::pktype> helper;
        odb::result<model::AstCountGroupByFilesJava> resultSet;

        model::HashType mangledNameHash = stoull(infoQuery.filters.front());
        model::JavaType javaType =
         query.queryEntityByHash<model::JavaType>(mangledNameHash);

        try
        {
          resultSet = db->query<model::AstCountGroupByFilesJava>(
            getUsageQuery(javaType, subQuery, helper));
        }
        catch(const empty_query_warning_exception &)
        {
          // this is a warning exception, just ignore it
        }

        return makeHitCountQueryNodes(db, resultSet, int(SubQuery::usageInFile),
          {std::to_string(int(subQuery)), std::to_string(mangledNameHash)});
      }
      break;

    case SubQuery::usageInFile:
      {
        std::vector<model::JavaAstNode::pktype> helper;

        if(3!=infoQuery.filters.size())
        {
          throw std::runtime_error("Not enough filter in InfoQuery object!");
        }

        SubQuery sub = SubQuery(stoul(infoQuery.filters[0]));
        model::FileId fileId = stoull(infoQuery.filters[2]);
        model::HashType mangledNameHash = std::stoull(infoQuery.filters[1]);

        model::JavaType javaType =
         query.queryEntityByHash<model::JavaType>(mangledNameHash);

        return getUsageOfInFile(
          getUsageQuery(javaType, sub, helper) &&
          JavaOdbQuery::astFileId(fileId));
      }
      break;

    case SubQuery::annotations:
      {
        return getAnnotationsFor(astNode);
      }
      break;

    case SubQuery::genericInst:
      {
        std::vector<InfoNode> ret;

        model::JavaType javaType =
         query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

        typedef odb::query<model::JavaType> QType;

        model::JavaType::pktype id = javaType.mangledNameHash;

        if(javaType.genericImpl.load())
        {
          id = javaType.genericImpl.load()->mangledNameHash;
        }

        for(const model::JavaType & inst : db->query<model::JavaType>(
          QType::genericImpl==id))
        {
          ret.push_back(makeInfoQueryNode({inst.qualifiedName, "Local"}, int(SubQuery::usageLocal),
            {std::to_string(inst.mangledNameHash)}));
          ret.push_back(makeInfoQueryNode({inst.qualifiedName, "Field"}, int(SubQuery::usageField),
            {std::to_string(inst.mangledNameHash)}));
          ret.push_back(makeInfoQueryNode({inst.qualifiedName, "Parameter"}, int(SubQuery::usageParam),
            {std::to_string(inst.mangledNameHash)}));
          ret.push_back(makeInfoQueryNode({inst.qualifiedName, "Return type"}, int(SubQuery::usageRetType),
            {std::to_string(inst.mangledNameHash)}));
        }

        return ret;
      }
      break;

    case SubQuery::typeArgs:
      {
        std::vector<InfoNode> ret;

        model::JavaType javaType =
         query.queryEntityByHash<model::JavaType>(astNode.mangledNameHash);

        typedef odb::query<model::JavaType_TypeParams> QTypePar;

        for(const model::JavaType_TypeParams & arg : db->query<model::JavaType_TypeParams>(
          QTypePar::type==javaType.mangledNameHash))
        {
          ret.push_back(makeInfoNodeFromType({}, arg.param.load()->name,
            *arg.param.load(), loadAstNodePtr(*arg.param.load(), false)));
        }

        return ret;
      }
      break;

    default:
      break;
    }

    return{};
  }
};

class VariableHandler : public SymbolHandler
{
  enum class SubQuery
  {
    annotations = 1,
    usage,
    usageInFile
  };
public:
  VariableHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::JavaAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG)<< "Trying to find JavaVariable with astNode id:"<< astNode.id;

    model::JavaVariable variable =
      query.queryEntityByHash<model::JavaVariable>(astNode.mangledNameHash);

    // short name with signature
    ret.push_back(makeInfoNode({}, "Name", variable.name));

    // qualified name
    ret.push_back(makeInfoNode({}, "Qualified Name", variable.qualifiedName));

    // type
    static const std::string label_Type("Type");
    if(std::shared_ptr<model::JavaType> type = variable.type.load())
    {
      ret.push_back(makeInfoNodeFromType({}, label_Type, *type, loadAstNodePtr(*type, false)));
    }
    else
    {
      ret.push_back(makeInfoNode({}, label_Type, "(external)"));
    }

    // declared
    SLog(util::DEBUG)<< "getting declaration for " << astNode.id;
    ret.push_back(makeInfoNode({}, "Declaration", getFileloc(astNode),
      createAstNodeInfo(astNode)));

    ret.push_back(makeInfoQueryNode({"Annotations"}, int(SubQuery::annotations)));
    ret.push_back(makeInfoQueryNode({"Reads"}, int(SubQuery::usage), {"r"}));
    ret.push_back(makeInfoQueryNode({"Writes"}, int(SubQuery::usage), {"w"}));

    SLog(util::DEBUG)<< "Returning from getInfoTreeForVariable";

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::JavaAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    model::AstType mode = model::AstType::Read;

    if(infoQuery.queryId!=int(SubQuery::annotations))
    {
      if(infoQuery.filters.empty())
      {
        throw std::runtime_error("No use-type (read or write) in InfoQuery.filters");
      }

      if(infoQuery.filters.front()=="w")
      {
        mode = model::AstType::Write;
      }
    }

    model::JavaVariable variable =
      query.queryEntityByHash<model::JavaVariable>(astNode.mangledNameHash);

    switch(static_cast<SubQuery>(infoQuery.queryId))
    {
    case SubQuery::annotations:
      {
        return getAnnotationsFor(astNode);
      }
      break;

    case SubQuery::usage:
      {
        std::vector<std::string> filters;
        filters.push_back(infoQuery.filters.front());
        return getUsageOf(variable.mangledNameHash, mode, int(SubQuery::usageInFile), filters);
      }
      break;

    case SubQuery::usageInFile:
      {
        if(infoQuery.filters.size() != 2)
          throw std::runtime_error("Invalid InfoQuery.filters");

        auto fileId = stoull(infoQuery.filters[1]);
        return getUsageOfInFile(variable.mangledNameHash, mode, fileId);
      }
      break;
    }

    return {};
  }
};

class FunctionHandler : public SymbolHandler
{
  enum class SubQuery
  {
    annotations = 1,
    parameters,
    locals,
    calls,
    calledBy,
    calledByInFile,
    genericInst,
    typeArgs,
  };
public:
  FunctionHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::string getInfoBoxText(const model::JavaAstNode & astNode)
  {
    model::JavaFunction javaFunction = query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash);

    if(javaFunction.genericImpl.load() && javaFunction.genericImpl.load()->astNodePtr.load())
    {
      return SymbolHandler::getInfoBoxText(*javaFunction.genericImpl.load()->astNodePtr.load());
    }

    return SymbolHandler::getInfoBoxText(astNode);
  }

  std::vector<InfoNode> getInfoTree(const model::JavaAstNode& astNode) override
  {
    std::vector<InfoNode> ret;

    SLog(util::DEBUG) << "Trying to find JavaFunction with astNode id:"<< astNode.id;

    model::JavaFunction function =
      query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash);

    // short name with signature
    ret.push_back(makeInfoNode({}, "Name", function.name));

    // qualified name
    ret.push_back(makeInfoNode({}, "Qualified Name", function.qualifiedName));

    // signature name
    ret.push_back(makeInfoNode({}, "Signature", getFunctionSignature(function, true)));

    // return type
    static const std::string label_ReturnType("Return type");
    if(function.returnType.load())
    {
      model::JavaAstNodePtr node = loadAstNodePtr(*function.returnType, false);

      if(node)
      {
        ret.push_back(makeInfoNode({}, label_ReturnType, function.returnType->qualifiedName,
          createAstNodeInfo(*node)));
      }
      else
      {
        ret.push_back(makeInfoNode({}, label_ReturnType, function.returnType->qualifiedName));
      }
    }

    model::JavaAstNodePtr realDefNode = loadAstNodePtr(function);

    // declared
    SLog(util::DEBUG)<< "getting declaration for " << astNode.id;
    if(realDefNode && realDefNode.load()->file.load())
    {
      ret.push_back(
        makeInfoNode({}, "Declaration", getFileloc(*realDefNode),
          createAstNodeInfo(*realDefNode)));
    }

    ret.push_back(makeInfoQueryNode({"Annotations"}, (int)SubQuery::annotations));
    ret.push_back(makeInfoQueryNode({"Parameters"}, (int)SubQuery::parameters));
    ret.push_back(makeInfoQueryNode({"Local Variables"}, (int)SubQuery::locals));
    ret.push_back(makeInfoQueryNode({"Calls"}, (int)SubQuery::calls));
    ret.push_back(makeInfoQueryNode({"Called by"}, (int)SubQuery::calledBy,
      {std::to_string(function.mangledNameHash)}));

    if(function.isGeneric || function.genericImpl.load())
    {
      ret.push_back(makeInfoQueryNode({"Generic Instances"}, int(SubQuery::genericInst)));
    }

    if(function.genericImpl.load())
    {
      ret.push_back(makeInfoQueryNode({"Type Arguments"}, int(SubQuery::typeArgs)));
    }

    SLog(util::DEBUG)<< "Returning from getInfoTreeForVariable";

    return ret;
  }

  std::vector<InfoNode> getVariableList(
      std::vector<odb::lazy_shared_ptr<model::JavaVariable>> & list_,
      bool sort)
  {
    std::vector<InfoNode> ret;

    for(odb::lazy_shared_ptr<model::JavaVariable> & var : list_)
    {
      if(var.load())
      {
        ret.push_back(makeInfoNodeFromType({}, var->name, *var->type.load(),
          var->astNodePtr.load()));
      }
    }

    if(sort)
    {
      std::sort(ret.begin(), ret.end(), [](const InfoNode& lhs, const InfoNode& rhs)
        {
          return lhs.label < rhs.label;
        });
    }

    return ret;
  }

  std::set<JavaHashType> extractAllOverrideHashes(const model::JavaFunction & function)
  {
    std::set<JavaHashType> hashes;

    {
      std::vector<JavaHashType> func_hashes = getOverrideHashes(db, query, function);
      hashes.insert(func_hashes.begin(), func_hashes.end());
    }

    if(function.isGeneric)
    {
      typedef odb::query<model::JavaFunction> QFunc;

      for(const model::JavaFunction & inst : db->query<model::JavaFunction>(
        QFunc::genericImpl==function.mangledNameHash))
      {
        std::vector<JavaHashType> inst_hashes = getOverrideHashes(db, query, inst);
        hashes.insert(inst_hashes.begin(), inst_hashes.end());
      }
    }

    return hashes;
  }

  std::vector<InfoNode> getCalledBy(const InfoQuery& infoQuery, model::JavaFunction function, model::HashType mangledNameHash)
  {
    if(function.mangledNameHash!=mangledNameHash)
    {
      function = query.queryEntityByHash<model::JavaFunction>(mangledNameHash);
    }

    std::set<JavaHashType> hashes = extractAllOverrideHashes(function);

    odb::result<model::AstCountGroupByFilesJava> resultSet =
      db->query<model::AstCountGroupByFilesJava>(
      (
        JavaOdbQuery::astAstType(model::AstType::Usage)
        ||
        JavaOdbQuery::astAstType(model::AstType::VirtualCall)
      )
      && odb::query<model::JavaAstNode>::mangledNameHash.in_range(hashes.begin(), hashes.end()));

    return makeHitCountQueryNodes(db, resultSet, int(SubQuery::calledByInFile), infoQuery.filters);
  }

  std::vector<InfoNode> getSubInfoTree(const model::JavaAstNode& astNode,
    const InfoQuery& infoQuery) override
  {
    model::JavaFunction function =
      query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash);

    switch(static_cast<SubQuery>(infoQuery.queryId))
    {
    case SubQuery::annotations:
      {
        return getAnnotationsFor(astNode);
      }
      break;

    case SubQuery::parameters:
      {
        return getVariableList(function.parameters, false);
      }
      break;

    case SubQuery::locals:
      {
        return getVariableList(function.locals, true);
      }
      break;

    case SubQuery::calls:
      {
        std::vector<InfoNode> ret;

        for(model::JavaAstNode & call : query.sortQuery<model::JavaAstNode>
          (JavaOdbQuery::queryCallsInAstNode(astNode), sortByPosition))
        {
          try
          {
            model::JavaFunction func =
              query.queryEntityByHash<model::JavaFunction>(call.mangledNameHash);

            ret.push_back(makeInfoNodeFromCall(
              {getFunctionSignature(func, false)}, func, call));
          }
          catch(const std::exception & ex)
          {
            SLog(util::ERROR)
              << "Exception caught while processing "
              << call.astValue
              << ", Message: " << ex.what();
          }
        }

        return ret;
      }
      break;

    case SubQuery::calledBy:
      {
        if(infoQuery.filters.empty())
        {
          throw std::runtime_error("Empty InfoQuery::filters in FunctionHandler::calledBy");
        }

        return getCalledBy(infoQuery, function, stoull(infoQuery.filters.front()));
      }
      break;

    case SubQuery::calledByInFile:
      {
        std::vector<InfoNode> ret;

        if(infoQuery.filters.empty())
          throw std::runtime_error("No file ID in InfoQuery.filters");

        model::HashType mangledNameHash = std::stoull(infoQuery.filters[0]);

        std::set<JavaHashType> hashes = extractAllOverrideHashes(
          query.queryEntityByHash<model::JavaFunction>(mangledNameHash));

        std::unordered_set<model::HashType> functionHashes;

        auto fileId = std::stoull(infoQuery.filters[1]);

        SLog(util::DEBUG) << "getting callers for " << astNode.id;
        for(auto caller : query.sortQuery<model::JavaAstNode>(
          (
            JavaOdbQuery::astAstType(model::AstType::Usage)
            ||
            JavaOdbQuery::astAstType(model::AstType::VirtualCall)
          )
          && JavaOdbQuery::astSymbolType(model::SymbolType::Function)
          && odb::query<model::JavaAstNode>::mangledNameHash.in_range(hashes.begin(), hashes.end())
          && JavaOdbQuery::astFileId(fileId), sortByPosition))
        {
          try
          {
            JavaHashType outerHash = query.loadOuterFunction(caller).mangledNameHash;
            model::JavaFunction outerFunc = query.queryEntityByHash<model::JavaFunction>(outerHash);

            if(0==functionHashes.count(outerHash))
            {
              functionHashes.insert(outerHash);

              ret.push_back(makeInfoQueryNode({outerFunc.signature, {"Called by"}}, int(SubQuery::calledBy),
                {std::to_string(outerHash)}));
            }

            ret.push_back(
                makeInfoNodeFromCall({}, outerFunc, caller));
          }
          catch (const std::exception& ex)
          {
            SLog(util::ERROR)
              << "Exception caught while processing "
              << caller.astValue
              << ", Message: " << ex.what();
          }
        }

        return ret;
      }
      break;

    case SubQuery::genericInst:
      {
        std::vector<InfoNode> ret;

        model::JavaFunction javaFunc =
         query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash);

        typedef odb::query<model::JavaFunction> QFunc;

        model::JavaFunction::pktype id = javaFunc.mangledNameHash;

        if(javaFunc.genericImpl.load())
        {
          id = javaFunc.genericImpl.load()->mangledNameHash;
        }

        for(const model::JavaFunction & inst : db->query<model::JavaFunction>(
          QFunc::genericImpl==id))
        {
          ret.push_back(makeInfoNode({}, "Instance", inst.mangledName));
        }

        return ret;
      }
      break;

    case SubQuery::typeArgs:
      {
        std::vector<InfoNode> ret;

        model::JavaFunction javaFunction =
         query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash);

        typedef odb::query<model::JavaFunction_TypeParams> QTypePar;

        for(const model::JavaFunction_TypeParams & arg : db->query<model::JavaFunction_TypeParams>(
          QTypePar::function==javaFunction.mangledNameHash))
        {
          ret.push_back(makeInfoNodeFromType({}, arg.param.load()->name,
            *arg.param.load(), loadAstNodePtr(*arg.param.load(), false)));
        }

        return ret;
      }
      break;
    }

    return {};
  }
};

class DefaultHandler : public SymbolHandler
{
public:
  DefaultHandler(std::shared_ptr<odb::database> db)
    : SymbolHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(const model::JavaAstNode& astNode) override
  {
    std::vector<InfoNode> ret;
    auto astInfo = createAstNodeInfo(astNode);

    ret.push_back(makeInfoNode({}, "ASTNode", ""));
    ret.push_back(
      makeInfoNode({}, astInfo.astNodeType, astNode.astValue));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::JavaAstNode& astNode,
    const InfoQuery& query) override
  {
    return
    {};
  }

  std::string getInfoBoxText(const model::JavaAstNode& astNode) override
  {
    return astNode.astValue;
  }
};

std::string SymbolHandler::getInfoBoxText(const model::JavaAstNode& astNode)
{
  auto defNode = query.loadDefinitionOrDeclaration(astNode);
  const auto& fileContent =
    defNode.file.load()->content.load()->content;
  model::Range range(
    model::Position(defNode.loc_start_line, defNode.loc_start_col),
    model::Position(defNode.loc_end_line, defNode.loc_end_col));
  return textRange(fileContent, range);
}

std::unique_ptr<SymbolHandler> SymbolHandler::getHandler(
  std::shared_ptr<odb::database> db, model::SymbolType symbolType)
{
  switch(symbolType)
  {
  case model::SymbolType::Variable:
    return std::unique_ptr<SymbolHandler> { new VariableHandler(db) };
  case model::SymbolType::Function:
    return std::unique_ptr<SymbolHandler> { new FunctionHandler(db) };
  case model::SymbolType::Type:
  case model::SymbolType::Enum:
    return std::unique_ptr<SymbolHandler> { new TypeHandler(db) };
  default: break;
  }
  return std::unique_ptr<SymbolHandler> { new DefaultHandler(db) };
}

std::vector<InfoNode> JavaFileHandler::getImports(model::FileId fid_)
{
  typedef odb::query<model::JavaImport> QImport;

  std::vector<InfoNode> ret;

  for(model::JavaImport & im : db->query<model::JavaImport>(QImport::file == fid_))
  {
    ret.push_back(SymbolHandler::makeInfoNodeEx({}, im.name, im.astNodePtr.load()));
  }

  return ret;
}

/**
 * This function returns the package name from a given type name. A name is a
 * typename if it begins with an uppercase letter. The package name in "a.b.C"
 * is "a.b".
 */
std::string JavaFileHandler::getPackageNameFromTypeName(const std::string & typename_)
{
  for(std::size_t pos=0,len=typename_.length();pos<len;++pos)
  {
    if(std::isupper(typename_[pos]))
    {
      if(0==pos)
      {
        return "";
      }

      return typename_.substr(0, pos-1);
    }
  }

  // No upper-case letter in the string.
  return typename_;
}

std::vector<InfoNode> JavaFileHandler::getTypes(model::FileId fid_)
{
  // query a Type to predict common prefix

  odb::result<model::JavaAstNode> types = db->query<model::JavaAstNode>(
    JavaOdbQuery::astFileId(fid_)
    &&
    JavaOdbQuery::astSymbolType(model::SymbolType::Type)
    &&
    (
      JavaOdbQuery::astAstType(model::AstType::Definition)
      ||
      JavaOdbQuery::astAstType(model::AstType::Declaration)
    )
  );

  if(types.empty())
  {
    // There is no type defined in this class.
    return {};
  }

  std::string prefix = "";//getPackageNameFromTypeName(types.begin()->mangledName);
  std::size_t prefixlen = 0;
  std::vector<std::string> cat;

  if(!prefix.empty())
  {
    cat.push_back(prefix);
    prefixlen = prefix.length()+1;
  }

  // query Types and Functions

  std::vector<InfoNode> ret = queryAstNodes(
    JavaOdbQuery::astFileId(fid_)
    && (
      JavaOdbQuery::astSymbolType(model::SymbolType::Type)
      ||
      JavaOdbQuery::astSymbolType(model::SymbolType::Function)
    )
    &&
    (
      JavaOdbQuery::astAstType(model::AstType::Definition)
      ||
      JavaOdbQuery::astAstType(model::AstType::Declaration)
    ),
    cat,
    prefixlen, true);

  std::vector<InfoNode> members = queryMembersOfTypes(
    types,
    cat,
    prefixlen,
    true);

  ret.insert(ret.end(), members.begin(), members.end());

  return ret;
}

/**
 * This function splits the given string str_ by dot character (.), if that
 * character is not between angle brackets or parentheses, starting from start_
 * position. The splitted parts are pushed in cat_ vector, except the last one.
 * E.g. if str_ is a.b.c<d.e>.f(g.h).i.j then these are going to be inserted to
 * the cat_: a, b, c<d.e>, f(g.h), i. 'j' is not inserted in cat_, since it is
 * the last part.
 * TODO: Hopefully in the beginning start_ is not the position of a dot
 * character, and there are no two dot characters next to each other, but in
 * real Java qualified names this can't happen (this function is passed
 * qualified names).
 */
void JavaFileHandler::splitToCategories(const std::string & str_,
  std::size_t start_,
  std::vector<std::string> & cat_)
{
  std::size_t start = start_;
  std::size_t dot_pos = start_;
  std::size_t length = str_.length();

  while((dot_pos=findNext(str_, '.', start+1))<length)
  {
    cat_.push_back(str_.substr(start, dot_pos-start));
    start = dot_pos + 1;
  }
}

/**
 * This function returns the position of the next ch_ character in str_ string,
 * which is not between angle brackets (<...>) or parentheses ((...)), because
 * these are skipped. The search starts at the position given by pos_. If the
 * character is not found, then the returning number is the length of str_, or
 * the given pos_ if it's greater or equal to the length of str_.
 */
std::size_t JavaFileHandler::findNext(const std::string & str_,
  char ch_, std::size_t pos_)
{
  std::size_t len = str_.length();

  for(;pos_<len && str_[pos_]!=ch_;++pos_)
  {
    if('<'==str_[pos_])
    {
      pos_ = findNext(str_, '>', pos_+1);
    }
    else
    if('('==str_[pos_])
    {
      pos_ = findNext(str_, ')', pos_+1);
    }
  }

  return pos_;
}

/**
 * This function queries the JavaAstNode records from database which meet the
 * astQuery_, and creates a vector of InfoNode objects which can be later
 * displayed in the InfoTree. The category is a vector which describes the path
 * of the node in the tree. The category_ parameter has to contain one element
 * (the root), of which the length is given by prefixlen_.
 */
std::vector<InfoNode> JavaFileHandler::queryAstNodes(
  JavaOdbQuery::AstQuery astQuery_,
  const std::vector<std::string>& category_,
  std::size_t prefixlen_,
  bool split_)
{
  std::vector<InfoNode> ret;

  for(model::JavaAstNode & node :
    db->query<model::JavaAstNode>(astQuery_ + "ORDER BY" + JavaOdbQuery::AstQuery::astValue))
  {
    // split qualified name to category hierarchy
    std::string qualifiedName;
    try {
      qualifiedName = query.getQualifiedName(node);
    } catch (std::runtime_error err) {
      // TODO: Nested classes in Java has $1 postfix, and these are not in the database
      continue;
    }

    std::vector<std::string> cat = category_;

    if(split_)
    {
      splitToCategories(qualifiedName, prefixlen_, cat);
    }

    std::string info_value = node.astValue;

    if(model::SymbolType::Function==node.symbolType)
    {
      try
      {
        model::JavaFunction func = query.queryEntityByAstNode<model::JavaFunction>(node);
        info_value = func.signature;

        if(func.returnType.load())
        {
          info_value += " : " + func.returnType->qualifiedName;
        }
      }
      catch(...)
      {
        // in case of any problem, just keep the 'info_value'
      }
    }

    // return with info node
    ret.push_back(SymbolHandler::makeInfoNode(cat, "", info_value,
      createAstNodeInfo(node)));
  }

  return ret;
}

std::vector<InfoNode> JavaFileHandler::queryMembersOfTypes(
  odb::result<model::JavaAstNode> types_,
  const std::vector<std::string>& category_,
  std::size_t prefixlen_,
  bool split_)
{
  typedef odb::query<model::JavaMember> QMember;

  std::vector<InfoNode> ret;

  // 'types_' contains only 1 item in most of the cases
  for(const model::JavaAstNode & node : types_)
  {
    try
    {
      model::JavaType type = query.queryEntityByAstNode<model::JavaType>(node);

      for(model::JavaMember & member :
        db->query<model::JavaMember>(QMember::type==type.mangledNameHash))
      {
        if(member.astNodePtr.load())
        {
          // split qualified name to category hierarchy
          std::string qualifiedName = member.astNodePtr->mangledName;
          std::vector<std::string> cat = category_;

          if(split_)
          {
            splitToCategories(qualifiedName, prefixlen_, cat);
          }

          std::string info_value = member.name;
          if(member.fieldType.load())
          {
            info_value += " : " + member.fieldType->qualifiedName;
          }

          // return with info node
          ret.push_back(SymbolHandler::makeInfoNode(cat, "", info_value,
            createAstNodeInfo(*member.astNodePtr)));
        }
      }
    }
    catch(...)
    {
      // ignore all exceptions
    }
  }

  return ret;
}

std::vector<InfoNode> JavaFileHandler::getInfoTreeForFile(
  const model::FileId fid)
{
  using namespace model;
  std::vector<InfoNode> ret;

  SLog(util::DEBUG)<< "Trying to find File with file id:" << fid;

  File file;
  db->load<File>(fid, file);

  // filename
  ret.push_back(SymbolHandler::makeInfoNode({}, "Name", file.filename));

  // path
  ret.push_back(SymbolHandler::makeInfoNode({}, "Path", file.path));

  // content
  ret.push_back(SymbolHandler::makeInfoNode({}, "Content", "Java source"));

  ret.push_back(SymbolHandler::makeInfoQueryNode({"Imports"}, (int)SubQuery::imports));
  ret.push_back(SymbolHandler::makeInfoQueryNode({"Types"}, (int)SubQuery::types));

  return ret;
}

std::vector<InfoNode> JavaFileHandler::getSubInfoTreeForFile(
  const model::FileId& fid, const InfoQuery& infoQuery)
{
  switch(static_cast<SubQuery>(infoQuery.queryId))
  {
    case SubQuery::imports: return getImports(fid);
    case SubQuery::types: return getTypes(fid);
    default:
      break;
  }

  return {};
}

} // language
} // service
} // cc

