/*
 * $Id$
 *
 *  Created on: Apr 7, 2014
 *      Author: ezoltbo
 */

#include "cxxparser/cxxastpersister.h"

#include "model/cxx/cppastnode-odb.hxx"
#include "model/cxx/cppenum-odb.hxx"
#include "model/cxx/cppfunction-odb.hxx"
#include "model/cxx/cppmacroexpansion-odb.hxx"
#include "model/cxx/cppnamespace-odb.hxx"
#include "model/cxx/cpptypedef-odb.hxx"
#include "model/cxx/cppvariable-odb.hxx"
#include "model/cxx/cppentity-odb.hxx"
#include "model/cxx/cppfunctionpointer-odb.hxx"
#include "model/cxx/cppmacro-odb.hxx"
#include "model/cxx/cpptype-odb.hxx"
#include "model/cxx/cppimplicit-odb.hxx"

#include "util/util.h"

namespace cc
{
namespace parser
{

using namespace cc::model;

CxxAstPersister::CppEntityPersister::CppEntityPersister(CxxAstPersister& pers_)
  : _astPers(pers_)
{
}

void CxxAstPersister::CppEntityPersister::visit(CppEnum& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppEnumConstant& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppFunction& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppFunctionPointer& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppNamespace& ent_)
{
  if (_astPers._namespaceCache.find(ent_.qualifiedName) ==
      _astPers._namespaceCache.end())
  {
    _astPers._workspace->getDb()->persist(ent_);
    _astPers._namespaceCache.insert(ent_.qualifiedName);
  }
}

void CxxAstPersister::CppEntityPersister::visit(CppMacro& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppType& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppImplicit& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppTypedef& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

void CxxAstPersister::CppEntityPersister::visit(CppVariable& ent_)
{
  _astPers._workspace->getDb()->persist(ent_);
}

CxxAstPersister::CxxAstPersister(std::shared_ptr<model::Workspace> w_) :
  _workspace(w_),
  _astCache(10000019),
  _memberCache(200131),
  _entAstCache(10000019),
  _entPers(*this)
{
  fillAstCache();
}

const model::CppNamespace& CxxAstPersister::getGlobalNamespace()
{
  return _globalNs;
}

void CxxAstPersister::persistAstNodes(CxxAstPersister::AstNodeMap& nodes_)
{
  if (nodes_.empty())
  {
    return;
  }

  std::unique_lock<std::mutex> lock(_astPersistMutex);
  {
    auto it = nodes_.begin();
    while (it != nodes_.end())
    {
      if (persistNode(*it->second))
      {
        ++it;
      }
      else
      {
        it = nodes_.erase(it);
      }
    }
  }
}

void CxxAstPersister::persistAstNodes(CxxAstPersister::AstNodeVector& nodes_)
{
  if (nodes_.empty())
  {
    return;
  }

  std::unique_lock<std::mutex> lock(_astPersistMutex);
  {
    auto it = nodes_.begin();
    while (it != nodes_.end())
    {
      if (persistNode(**it))
      {
        ++it;
      }
      else
      {
        it = nodes_.erase(it);
      }
    }
  }
}

void CxxAstPersister::fillAstCache()
{
  SLog(util::STATUS) << "Filling C++ AST cache...";

  std::unique_lock<std::mutex> lock(_astPersistMutex);
  std::unique_lock<std::mutex> lock2(_entPersistMutex);

  util::OdbTransaction trans(*_workspace->getDb());
  trans([&, this]
  {
    odb::database& db = *_workspace->getDb();

    for (auto astNodeId : db.query<model::CppAstNodeId>())
    {
      _astCache.insert(astNodeId.id);
    }

    for (auto entAstId : db.query<model::CppEntityAstNodeId>())
    {
      if (!entAstId.astNodeId.null())
      {
        _entAstCache.insert(entAstId.astNodeId.get());
      }
    }

    {
      // Get namespaces
      auto allNs = db.query<model::CppNamespace>();
      for (const model::CppNamespace& ns : allNs)
      {
        if (ns.qualifiedName == model::CppGlobalNamespace)
        {
          _globalNs = ns;
        }

        _namespaceCache.insert(ns.qualifiedName);
      }

      // Create global namespace if not found
      if (_globalNs.qualifiedName.empty())
      {
        _globalNs.name = model::CppGlobalNamespace;
        _globalNs.qualifiedName = model::CppGlobalNamespace;
        _globalNs.mangledNameHash = util::fnvHash(_globalNs.qualifiedName);
        db.persist(_globalNs);
      }
    }
  });
}

bool CxxAstPersister::persistNode(model::CppAstNode& astNode)
{
  auto id = createIdentifier(astNode);
  auto hash = util::fnvHash(id);

  astNode.id = hash;
  astNode.mangledNameHash = util::fnvHash(astNode.mangledName);
  if (_astCache.find(hash) != _astCache.end())
  {
    return false;
  }

  _workspace->getDb()->persist(astNode);
  _astCache.insert(hash);

  return true;
}

bool CxxAstPersister::persistEntity(model::CppEntity& ent_)
{
  // The astNodePtr could be NULL for an entity
  if (ent_.astNodeId &&
      _entAstCache.find(ent_.astNodeId.get()) != _entAstCache.end())
  {
    return false;
  }

  ent_.accept(_entPers);
  if (ent_.astNodeId)
  {
    _entAstCache.insert(ent_.astNodeId.get());
  }

  return true;
}

bool CxxAstPersister::persistEntity(model::CppMemberType& ent_)
{
  const auto& id = ent_.memberAstNode.object_id();
  if (_memberCache.find(id) != _memberCache.end())
  {
    return false;
  }

  _workspace->getDb()->persist(ent_);
  _memberCache.insert(id);

  return true;
}

std::string CxxAstPersister::createIdentifier(
  const model::CppAstNode& astNode)
{
  using SymbolTypeInt =
    std::underlying_type<model::CppAstNode::SymbolType>::type;
  using AstTypeInt =
    std::underlying_type<model::CppAstNode::AstType>::type;

  std::string res =
    astNode.astValue    + ':' +
    astNode.mangledName + ':' +
    std::to_string(static_cast<SymbolTypeInt>(astNode.symbolType)) + ':' +
    std::to_string(static_cast<AstTypeInt>(astNode.astType)) + ':' +
    std::to_string(astNode.visibleInSourceCode) + ':';

  if (astNode.location.file)
  {
    res += std::to_string(astNode.location.file.object_id()) + ':';
    res += std::to_string(astNode.location.range.start.line) + ':';
    res += std::to_string(astNode.location.range.start.column) + ':';
    res += std::to_string(astNode.location.range.end.line) + ':';
    res += std::to_string(astNode.location.range.end.column) + ':';
  }
  else
  {
    res += "null";
  }

  return res;
}

} // parser
} // cc
