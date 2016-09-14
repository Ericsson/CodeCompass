#ifndef CODECOMPASS_MODEL_BUILD_ACTION_H
#define CODECOMPASS_MODEL_BUILD_ACTION_H

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/project.h>
#include <model/buildparameter.h>
#include <model/buildtarget.h>
#include <model/buildsource.h>
#include <model/buildlog.h>

namespace cc
{
namespace model
{

struct BuildAction;
struct BuildParameter;
struct BuildSource;
struct BuildTarget;
struct BuildLog;

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

  using pktype = uint64_t;

  #pragma db id
  pktype id;

  #pragma db not_null
  std::string label;

  #pragma db not_null
  odb::lazy_shared_ptr<Project> project;

  #pragma db not_null
  unsigned int type;

  #pragma db not_null
  unsigned int state = StCreated;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildSource>> sources;

  #pragma db value_not_null inverse(action)
  std::vector<odb::lazy_weak_ptr<BuildTarget>> targets;
};

#pragma db view object(BuildAction)
struct BuildActionId
{
  uint64_t id;
};

#pragma db view object(BuildAction)
struct BuildActionIdState
{
  uint64_t id;

  unsigned int state;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_BUILD_ACTION_H
