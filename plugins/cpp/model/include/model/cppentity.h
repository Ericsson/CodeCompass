#ifndef CC_MODEL_CPPENTITY_H
#define CC_MODEL_CPPENTITY_H

#include <string>

#include <odb/nullable.hxx>

#include "cppastnode.h"
#include "common.h"

namespace cc
{
namespace model
{

typedef std::uint64_t CppEntityId;

#pragma db object polymorphic
struct CppEntity
{
  virtual ~CppEntity() {}

  #pragma db id auto
  CppEntityId id;

  #pragma db unique
  CppAstNodeId astNodeId;

  std::uint64_t mangledNameHash = 0;

  std::string name;
  std::string qualifiedName;

  std::set<Tag> tags;

#ifndef NO_INDICES
  #pragma db index member(mangledNameHash)
#endif
};

typedef std::shared_ptr<CppEntity> CppEntityPtr;

#pragma db object
struct CppTypedEntity : CppEntity
{
  std::uint64_t typeHash;
  std::string qualifiedType;
};

typedef std::shared_ptr<CppTypedEntity> CppTypedEntityPtr;

}
}

#endif
