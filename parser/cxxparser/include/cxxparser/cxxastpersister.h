/*
 * $Id$
 *
 *  Created on: Apr 7, 2014
 *      Author: ezoltbo
 */

#ifndef ASTPERSISTER_H_
#define ASTPERSISTER_H_

#include <memory>
#include <mutex>
#include <type_traits>

#include "util/odbobjectcache.h"

#include "model/workspace.h"
#include "model/cxx/cppastnode.h"
#include "model/cxx/cppentity.h"
#include "model/cxx/cpptype.h"
#include "model/cxx/cppnamespace.h"
#include "model/cxx/cppimplicit.h"

namespace cc
{
namespace parser
{

/**
 * Helper class for persisting the model for a C++ project.
 *
 * This class is thread safe.
 */
class CxxAstPersister
{
private:
  class CppEntityPersister : public model::CppEntityVisitor
  {
  public:
    CppEntityPersister(CxxAstPersister&);
  
    virtual void visit(model::CppEnum& ent_) override;
    virtual void visit(model::CppEnumConstant& ent_) override;
    virtual void visit(model::CppFunction& ent_) override;
    virtual void visit(model::CppFunctionPointer& ent_) override;
    virtual void visit(model::CppNamespace& ent_) override;
    virtual void visit(model::CppMacro& ent_) override;
    virtual void visit(model::CppType& ent_) override;
    virtual void visit(model::CppTypedef& ent_) override;
    virtual void visit(model::CppVariable& ent_) override;
    virtual void visit(model::CppImplicit& ent_) override;

  private:
    CxxAstPersister& _astPers;
  };

public:
  using AstNodeMap = std::map<const void*, model::CppAstNodePtr>;
  using AstNodeVector = std::vector<model::CppAstNodePtr>;

  CxxAstPersister(std::shared_ptr<model::Workspace> w_);

  void persistAstNodes(AstNodeMap& nodes_);

  void persistAstNodes(AstNodeVector& nodes_);

  template <typename ContainerT>
  void persistEntities(ContainerT& items_)
  {
    if (items_.empty())
    {
      return;
    }

    std::unique_lock<std::mutex> lock(_entPersistMutex);
    {
      for (auto& item : items_)
      {
        persistEntityItem(item);
      }
    }
  }

  const model::CppNamespace& getGlobalNamespace();
  
  std::string createIdentifier(const model::CppAstNode& astNode_);
private:
  void fillAstCache();

  bool persistNode(model::CppAstNode& node_);

  bool persistEntity(model::CppEntity& ent_);
  //FIXME: CppMemberType is not an entity.
  bool persistEntity(model::CppMemberType& ent_);

  template <typename KeyT, typename PtrT>
  bool persistEntityItem(std::pair<KeyT, PtrT>& pair_)
  {
    return persistEntity(*pair_.second);
  }

  template <typename PtrT>
  bool persistEntityItem(PtrT& ptr_)
  {
    return persistEntity(*ptr_);
  }
  
  std::shared_ptr<model::Workspace> _workspace;

  std::mutex _astPersistMutex;
  std::mutex _entPersistMutex;
  model::AstCacheType _astCache;
  model::AstCacheType _memberCache;
  model::AstCacheType _entAstCache;
  std::unordered_set<std::string> _namespaceCache;
  
  CppEntityPersister _entPers;

  model::CppNamespace _globalNs;
};

} // parser
} // cc


#endif /* ASTPERSISTER_H_ */
