#ifndef CC_MODEL_BUILDACTION_H
#define CC_MODEL_BUILDACTION_H

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/common.h>
#include <model/buildtarget.h>
#include <model/buildsource.h>

namespace cc
{
namespace model
{

struct BuildAction;
struct BuildSource;
struct BuildTarget;

typedef std::shared_ptr<BuildAction> BuildActionPtr;

#pragma db object
struct BuildAction
{
  enum Type
  {
    Compile,
    Link,
    Other
  };

  enum State
  {
    StCreated,
    StParsed,
    StSkipped
  };

  #pragma db id
  pktype id;

  #pragma db not_null
  std::string command;

  #pragma db not_null
  Type type;

  #pragma db not_null
  State state = StCreated;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildSource>> sources;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildTarget>> targets;
};

#pragma db view object(BuildAction)
struct BuildActionId
{
  pktype id;
};

#pragma db view object(BuildAction)
struct BuildActionIdState
{
  pktype id;

  BuildAction::State state;
};

} // model
} // cc

#endif // CC_MODEL_BUILDACTION_H
