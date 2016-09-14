// $Id$
// Created by Aron Barath, 2013

#ifndef LANGSERVICELIB_ODBQUERY_H
#define LANGSERVICELIB_ODBQUERY_H

#include <memory>
#include <set>
#include <queue>

#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/database.hxx>

#include <model/file.h>
#include <model/position.h>
#include <model/asttype.h>

namespace cc
{
namespace service
{
namespace language
{

template<class AstNode, typename HashType, class LangType, class LangEnum>
class OdbQuery
{
public:

  typedef odb::query<AstNode> AstQuery;
  typedef odb::result<AstNode> AstResult;

  // I didn't find any other thread-safe way
  // of parameterizing odb queries

  static AstQuery astMangledNameHash(const HashType hash)
  {
    return {AstQuery::mangledNameHash == hash};
  }

  static AstQuery astAstType(const model::AstType astType)
  {
    return {AstQuery::astType == astType};
  }

  static AstQuery astSymbolType(const model::SymbolType symbolType)
  {
    return {AstQuery::symbolType == symbolType};
  }

  static AstQuery queryCallsInAstNode(const AstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
      (astAstType(model::AstType::Usage) ||
       astAstType(model::AstType::VirtualCall) ) &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryStaticCallsInAstNode(const AstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
      astAstType(model::AstType::Usage) &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryDynamicCallsInAstNode(const AstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
       astAstType(model::AstType::VirtualCall) &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryCallers(const AstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      (astAstType(model::AstType::Usage) ||
       astAstType(model::AstType::VirtualCall)
      )
      &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryStaticCallers(const AstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::AstType::Usage) &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryDynamicCallers(const AstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
       astAstType(model::AstType::VirtualCall) &&
      astSymbolType(model::SymbolType::Function);
  }

  static AstQuery queryReads(const AstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::AstType::Read);
  }

  OdbQuery(std::shared_ptr<odb::database> db)
  : db(db)
  {
  }

  template<typename T, typename Compare>
  std::vector<T> sortQuery(odb::query<T> query, Compare comp)
  {
    auto dbResult = db->query<T>(query);

    std::vector<T> result(dbResult.begin(), dbResult.end());

    std::sort(result.begin(), result.end(), comp);

    return result;
  }

  AstNode loadAstNode(uint64_t id) const
  {
    AstNode ret;

    db->load<AstNode>(id, ret);

    return ret;
  }

  AstNode loadAstNode(const std::string& id) const
  {
    return loadAstNode(stoull(id));
  }

  AstNode loadDefinition(const AstNode& astNode) const
  {
    auto result = db->query<AstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::AstType::Definition));

    if(result.empty())
      throw std::runtime_error("No definition for AST mangled name hash: "
        + std::to_string(astNode.mangledNameHash));

    return *result.begin();
  }

  AstNode loadDefinitionOrDeclaration(const AstNode& astNode) const
  {
    if(astNode.astType == model::AstType::Definition)
      return astNode;

    auto result = db->query<AstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::AstType::Definition));

    if(!result.empty())
      return *result.begin();

    auto decls = db->query<AstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::AstType::Declaration));

    if(decls.empty())
    {
      throw NoDefintionNorDeclaration(
        "No declaration for " + std::to_string(astNode.id));
    }

    return *decls.begin();
  }

  AstNode loadOuterFunction(const AstNode& astNode) const
  {
    auto result = db->query<AstNode>(
      astFileId(getFileId(astNode)) &&
      astContains(getRange(astNode).start) &&
      astAstType(model::AstType::Definition)
    );

    for(auto node : result)
    {
      if(node.symbolType == model::SymbolType::Function)
        return node;
    }

    throw NoOuterFunction(
      std::string("No outer function found for ") + std::to_string(astNode.id));
  }

  std::string getTypeOrEnumName(const HashType hash) const
  {
    using namespace cc::model;
    typedef odb::query<LangType> TQuery;
    typedef odb::query<LangEnum> EQuery;

    std::string typeName;

    auto tRes = db->query<LangType>(TQuery::mangledNameHash == hash);

    if(!tRes.empty())
    {
      typeName = tRes.begin()->name;
    }
    else
    {
      auto eRes = db->query<LangEnum>(EQuery::mangledNameHash == hash);

      if(!eRes.empty())
      {
        typeName = eRes.begin()->name;
      }
    }

    return typeName;
  }

  AstNode loadTypeAstNode(const HashType hash) const
  {
    using namespace cc::model;
    typedef odb::query<LangType> TQuery;
    typedef odb::query<LangEnum> EQuery;

    std::string typeName;

    {
      auto tRes = db->query<LangType>(TQuery::mangledNameHash == hash);

      if(!tRes.empty())
      {
        auto& type = *tRes.begin();
        return *type.astNodePtr.load();
      }
    }

    {
      auto eRes = db->query<LangEnum>(EQuery::mangledNameHash == hash);

      if(!eRes.empty())
      {
        auto& type = *eRes.begin();
        return *type.astNodePtr.load();
      }
    }

    return {};
  }

  template<typename DocCommentType>
  std::vector<DocCommentType> loadDocComments(const HashType hash) const
  {
    std::vector<DocCommentType> ret;
    typedef odb::query<DocCommentType> DcQuery;

    {
      auto tRes = db->query<DocCommentType>(DcQuery::mangledNameHash == hash);

      if(!tRes.empty())
      {
        ret.push_back(*tRes.begin());
      }
    }

    return ret;
  }

  template<typename Relation>
  std::set<HashType> getTransitiveClosureOfRel
    (const typename Relation::Kind kind, const HashType from) const
  {
    using namespace cc::model;
    typedef typename odb::query<Relation> Query;

    std::set<HashType> ret;

    std::queue<HashType> q;
    q.push(from);

    while(!q.empty())
    {
      auto current = q.front();
      q.pop();

      auto result = db->query<Relation>(
        Query::lhs == current && Query::kind == kind);

      for(auto relation : result)
      {
        if(ret.find(relation.rhs) == ret.end())
        {
          ret.insert(relation.rhs);
          q.push(relation.rhs);
        }
      }
    }

    return ret;
  }

  template<typename Relation>
  std::set<HashType> reverseTransitiveClosureOfRel
    (const typename Relation::Kind kind, const HashType to) const
  {
    using namespace cc::model;
    typedef typename odb::query<Relation> Query;

    std::set<HashType> ret;

    std::queue<HashType> q;
    q.push(to);

    while(!q.empty())
    {
      auto current = q.front();
      q.pop();

      auto result = db->query<Relation>(
        Query::rhs == current && Query::kind == kind);

      for(auto relation : result)
      {
        if(ret.find(relation.lhs) == ret.end())
        {
          ret.insert(relation.lhs);
          q.push(relation.lhs);
        }
      }
    }

    return ret;
  }

  template<typename Entity>
  Entity queryEntityByHash(const HashType hash) const
  {
    typedef odb::query<Entity> EQuery;

    auto result = db->query<Entity>(
      EQuery::mangledNameHash == hash &&
      EQuery::astNodePtr->astType == model::AstType::Definition);

    if (!result.empty())
      return *result.begin();

    auto decls = db->query<Entity>(
      EQuery::mangledNameHash == hash);

    if (decls.empty())
      throw std::runtime_error("No Entity for hash " + std::to_string(hash));

    return *decls.begin();
  }

  template<typename Entity>
  Entity queryEntityByAstNode(const AstNode& astNode) const
  {
    typedef odb::query<Entity> EQuery;

    auto result = db->query<Entity>(
      EQuery::astNodePtr == astNode.id);

    if (result.empty())
      throw std::runtime_error("No Entity for AST node " + std::to_string(astNode.id));

    return *result.begin();
  }

  // additional functionalities

  template<class HeaderInclusionNode>
  class HeaderInclusion
  {
  public:
    typedef odb::query<HeaderInclusionNode> HIQuery;
    typedef odb::result<HeaderInclusionNode> HIResult;

    static HIQuery headerIncluder(const model::FileId fileId)
    {
      return {HIQuery::includer == fileId};
    }

    static HIQuery headerIncluded(const model::FileId fileId)
    {
      return {HIQuery::included == fileId};
    }
  };

  struct NoOuterFunction : std::runtime_error {
    NoOuterFunction(const std::string& msg) :
      std::runtime_error(msg)
    {}
  };
  struct NoDefintionNorDeclaration : std::runtime_error {
    NoDefintionNorDeclaration(const std::string& msg) :
        std::runtime_error(msg)
      {}
  };

  // these functions must be specialized

  template<class AstNode_=AstNode>
  static model::FileId getFileId(const AstNode & node)
  {
    "getFileId() must be specialized";
    return model::FileId();
  }

  template<class AstNode_=AstNode>
  static model::Range getRange(const AstNode & node)
  {
    "getRange() must be specialized";
    return model::Range();
  }

  template<class AstNode_=AstNode>
  static AstQuery astFileId(const model::FileId fileId)
  {
    "astFileId() must be specialized";
  }

  template<class AstNode_=AstNode>
  static AstQuery astInRange(const model::Range range)
  {
    "astInRange() must be specialized";
  }

  template<class AstNode_=AstNode>
  static AstQuery astContains(const model::Position position)
  {
    "astContains() must be specialized";
  }

  template<class AstNode_=AstNode>
  std::string getQualifiedName(const AstNode& astNode) const
  {
    return "getQualifiedName() must be specialized";
  }

private:
  std::shared_ptr<odb::database> db;
};

} // language
} // service
} // cc

#endif // LANGSERVICELIB_ODBQUERY_H

