#ifndef CC_MODEL_BUILDACTION_H
#define CC_MODEL_BUILDACTION_H

#include <string>
#include <vector>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/buildsourcetarget.h>

namespace cc
{
namespace model
{

struct BuildSource;
struct BuildTarget;

#pragma db object
struct BuildAction
{
  enum Type
  {
    Compile,
    Link,
    Other
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  std::string command;

  #pragma db not_null
  Type type;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildSource>> sources;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildTarget>> targets;
};

typedef std::shared_ptr<BuildAction> BuildActionPtr;

#pragma db view object(BuildAction)
struct BuildActionId
{
  std::uint64_t id;
};

} // model
} // cc

#endif // CC_MODEL_BUILDACTION_H
