/*
 * odbquery.h
 *
 *  Created on: Aug 28, 2013
 *      Author: ezoltbo
 */

#ifndef CPPSERVICEHELPER_ODBQUERY_H
#define CPPSERVICEHELPER_ODBQUERY_H

#include <queue>
#include <algorithm>

#include "model/comment/doccomment.h"
#include "model/comment/doccomment-odb.hxx"
#include "model/cxx/cppastnode.h"
#include "model/cxx/cppastnode-odb.hxx"
#include "model/cxx/cppheaderinclusion.h"
#include "model/cxx/cppheaderinclusion-odb.hxx"
#include "model/cxx/cpprelation.h"
#include "model/cxx/cpprelation-odb.hxx"
#include "model/cxx/cppentity.h"
#include "model/cxx/cppentity-odb.hxx"
 #include "model/cxx/cpptype.h"
#include "model/cxx/cpptype-odb.hxx"


namespace cc
{ 
namespace service
{  
namespace language
{

class CppOdbQuery
{
public:

  typedef odb::query<model::CppAstNode>  AstQuery;
  typedef odb::result<model::CppAstNode> AstResult;

  typedef model::HashType HashType;


  static AstQuery astMangledNameHash(const HashType hash)
  {
    return {AstQuery::mangledNameHash == hash};
  }

  static AstQuery astAstType(const model::CppAstNode::AstType astType)
  {
    return {AstQuery::astType == astType};
  }

  static AstQuery astSymbolType(const model::CppAstNode::SymbolType symbolType)
  {
    return {AstQuery::symbolType == symbolType};
  }

  static AstQuery queryCallsInAstNode(const model::CppAstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
      (astAstType(model::CppAstNode::AstType::Usage) ||
       astAstType(model::CppAstNode::AstType::VirtualCall) ) &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryStaticCallsInAstNode(const model::CppAstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
      astAstType(model::CppAstNode::AstType::Usage) &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryDynamicCallsInAstNode(const model::CppAstNode astNode)
  {
    return
      astFileId(getFileId(astNode)) &&
      astInRange(getRange(astNode)) &&
       astAstType(model::CppAstNode::AstType::VirtualCall) &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryCallers(const model::CppAstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      (astAstType(model::CppAstNode::AstType::Usage) ||
       astAstType(model::CppAstNode::AstType::VirtualCall)
      )
      &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryStaticCallers(const model::CppAstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::CppAstNode::AstType::Usage) &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryDynamicCallers(const model::CppAstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
       astAstType(model::CppAstNode::AstType::VirtualCall) &&
      astSymbolType(model::CppAstNode::SymbolType::Function);
  }

  static AstQuery queryReads(const model::CppAstNode astNode)
  {
    return astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::CppAstNode::AstType::Read);
  }

  CppOdbQuery(std::shared_ptr<odb::database> db)
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

  model::CppAstNode loadAstNode(HashType id) const
  {
    model::CppAstNode ret;
    db->load<model::CppAstNode>(id, ret);

    return ret;
  }

  model::CppAstNode loadAstNode(const std::string& id) const
  {
    return loadAstNode(stoull(id));
  }

  model::CppAstNode loadDefinition(const model::CppAstNode& astNode) const
  {
    auto result = db->query<model::CppAstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::CppAstNode::AstType::Definition));

    if(result.empty())
      throw std::runtime_error("No definition for AST mangled name hash: "
        + std::to_string(astNode.mangledNameHash));

    return *result.begin();
  }

  /**
   * This function returns a vector of definitions. If astNode itself is a
   * definition then the returning vector contains this node. Otherwise a vector
   * of definition nodes is returned of which the mangled name is identical to
   * the astNode's mangled name. If no such nodes found then a random
   * declaration node is returned in a vector of which the mangled name matches.
   * 
   * According to the "one defifition rule" a named value must have exactly one
   * definition. However it is not always simply decidable or unambiguous,
   * regarding that a definition can be compiled into more object files, and the
   * linking information is not always enough (e.g. the project is compiled to
   * two targets, each of which using a different definition of a given entity).
   * 
   * @param astNode A node of which we are looking for the definitions or
   * declaration.
   */
  std::vector<model::CppAstNode> loadDefinitionOrDeclaration(
    const model::CppAstNode& astNode) const
  {
    if(astNode.astType == model::CppAstNode::AstType::Definition)
      return {astNode};
    
    auto result = db->query<model::CppAstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::CppAstNode::AstType::Definition));

    std::vector<model::CppAstNode> definitions(result.begin(), result.end());
    
    if(astNode.astType == model::CppAstNode::AstType::VirtualCall) 
    {
      auto defHash = getTransitiveClosureOfRel(
        model::CppRelation::Kind::Override, astNode.mangledNameHash);

      for (const auto& hash : defHash)
      {
        for (auto ref : db->query<model::CppAstNode>(
                          astMangledNameHash(hash) && 
                          astAstType(model::CppAstNode::AstType::Definition)))
        {
          definitions.push_back(ref);
        }
      }
    }
    
    if(!definitions.empty())
      return definitions;

    auto decls = db->query<model::CppAstNode>(
      astMangledNameHash(astNode.mangledNameHash) &&
      astAstType(model::CppAstNode::AstType::Declaration));

    if(decls.empty())
    {
      throw NoDefintionNorDeclaration(
        "No declaration for " + std::to_string(astNode.id));
    }

    return {*decls.begin()};
  }

  model::CppAstNode loadOuterFunction(const model::CppAstNode& astNode) const
  {
    auto result = db->query<model::CppAstNode>(
      astFileId(getFileId(astNode)) &&
      astContains(getRange(astNode).start) &&
      astAstType(model::CppAstNode::AstType::Definition)
    );

    for(auto node : result)
    {
      if(node.symbolType == model::CppAstNode::SymbolType::Function)
        return node;
    }

    throw NoOuterFunction(
      std::string("No outer function found for ") + std::to_string(astNode.id));
  }

  std::vector<model::DocComment> loadDocComments(const HashType hash) /*const*/
  {
    std::vector<model::DocComment> ret;
    typedef odb::query<model::DocComment> DcQuery;

    {
      auto tRes = db->query<model::DocComment>(DcQuery::mangledNameHash == hash);

      if(!tRes.empty())
      {
        ret.push_back(*tRes.begin());
      }
    }

    return ret;
  }

  std::set<HashType> getTransitiveClosureOfRel
    (const model::CppRelation::Kind kind, const HashType from) const
  {
    using namespace cc::model;
    typedef typename odb::query<model::CppRelation> Query;

    std::set<HashType> ret;

    std::queue<HashType> q;
    q.push(from);

    while(!q.empty())
    {
      auto current = q.front();
      q.pop();

      auto result = db->query<model::CppRelation>(
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

  std::set<HashType> reverseTransitiveClosureOfRel
    (const model::CppRelation::Kind kind, const HashType to) const
  {
    using namespace cc::model;
    typedef typename odb::query<model::CppRelation> Query;

    std::set<HashType> ret;

    std::queue<HashType> q;
    q.push(to);

    while(!q.empty())
    {
      auto current = q.front();
      q.pop();

      auto result = db->query<model::CppRelation>(
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

  odb::result<model::CppEntity> queryEntityByHash(const HashType hash) const
  {
    typedef odb::query<model::CppEntityByHash> EAQuery;
    typedef odb::query<model::CppEntity> EQuery;

    auto result = db->query<model::CppEntityByHash>(
      EAQuery::CppEntity::mangledNameHash == hash &&
      EAQuery::CppAstNode::astType == model::CppAstNode::AstType::Definition);

    if (!result.empty())
    {
      return db->query<model::CppEntity>(
        EQuery::id == result.begin()->id);
    }

    auto decls = db->query<model::CppEntity>(
      EQuery::mangledNameHash == hash);

    if (decls.empty())
      throw std::runtime_error("No Entity for hash " + std::to_string(hash));

    return decls;
  }

  template<typename Entity>
  Entity queryEntityByHash(const HashType hash) const
  {
    typedef odb::query<model::CppEntityByHash> EAQuery;
    typedef odb::query<Entity> EQuery;

    auto entities = db->query<model::CppEntityByHash>(
      EAQuery::CppEntity::mangledNameHash == hash &&
      EAQuery::CppAstNode::astType == model::CppAstNode::AstType::Definition);

    if (!entities.empty())
    {
      // FIXME: it's not very elegant but the most probable scenario is that
      // the entities result set has only one element and it is the match.
      for (const model::CppEntityByHash& ebh : entities)
      {
        auto result = db->query<Entity>(
          EQuery::id == ebh.id);
        if (!result.empty())
        {
          return *result.begin();
        }
      }
    }

    auto decls = db->query<Entity>(
      EQuery::mangledNameHash == hash);

    if (decls.empty())
      throw std::runtime_error("No Entity for hash " + std::to_string(hash));

    return *decls.begin();
  }

  odb::result<model::CppEntity> queryEntityByAstNodeId(const model::CppAstNode::pktype& id) const
  {
    typedef odb::query<model::CppEntity> EQuery;

    auto result = db->query<model::CppEntity>(
      EQuery::astNodeId == id);

    if (result.empty())
      throw std::runtime_error("No Entity for AST node " + id);

    return result;
  }

  template<typename Entity>
  Entity queryEntityByAstNode(const model::CppAstNode& astNode) const
  {
    typedef odb::query<Entity> EQuery;

    auto result = db->query<Entity>(
      EQuery::astNodeId == astNode.id);

    if (result.empty())
      throw std::runtime_error("No Entity for AST node " + astNode.id);

    return *result.begin();
  }

  odb::result<model::CppMemberType> queryMembersByType(
    const model::CppEntity& cppType, const model::CppMemberType::Kind kind) const
  {
    typedef odb::query<model::CppMemberType> MTQuery;

    return db->query<model::CppMemberType>(
      MTQuery::typeHash == cppType.mangledNameHash &&
      MTQuery::kind == kind);
  }

  // additional functionalities
  class HeaderInclusion
  {
  public:
    typedef odb::query<model::CppHeaderInclusion>  HIQuery;
    typedef odb::result<model::CppHeaderInclusion> HIResult;

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

  static model::FileId getFileId(const model::CppAstNode & node)
  {
    return node.location.file.object_id();
  }

  static model::Range getRange(const model::CppAstNode & node)
  {
    return node.location.range;
  }

  static AstQuery astFileId(const model::FileId fileId)
  {
    return {AstQuery::location.file == fileId};
  }

  static AstQuery astInRange(const model::Range range)
  {
    const auto &start = range.start;
    const auto &end = range.end;

    return {
      // start <= pos
      ((AstQuery::location.range.start.line == start.line
          && start.column <= AstQuery::location.range.start.column)
        || start.line < AstQuery::location.range.start.line)
      &&
      // pos < end
      ((AstQuery::location.range.end.line == end.line
          && AstQuery::location.range.end.column < end.column)
        || AstQuery::location.range.end.line < end.line)};
  }

  static AstQuery astContains(const model::Position position)
  {
    const auto line = position.line;
    const auto column = position.column;

    return {
        // StartPos <= Pos
        ((AstQuery::location.range.start.line == line
            && AstQuery::location.range.start.column <= column)
          || AstQuery::location.range.start.line < line)
        &&
        // Pos < EndPos
        ((AstQuery::location.range.end.line == line
            && column < AstQuery::location.range.end.column)
          || line < AstQuery::location.range.end.line)
    };
  }
  
  static AstQuery astStartWithRange(const model::Position position)
  {
    const auto line = position.line;
    const auto column = position.column;

    return {
        (AstQuery::location.range.start.line == line
            && AstQuery::location.range.start.column == column) 
    };
  }

  std::string getQualifiedName(const model::CppAstNode& astNode) const
  {
    auto r = queryEntityByHash(astNode.mangledNameHash);

    auto& entity = *r.begin();

    return entity.qualifiedName;
  }

private:
  std::shared_ptr<odb::database> db;
};


} // language
} // service
} // cc

#endif // CPPSERVICEHELPER_ODBQUERY_H

