#ifndef CC_MODEL_BUILDSOURCE_H
#define CC_MODEL_BUILDSOURCE_H

#include <memory>

#include <odb/core.hxx>

#include <model/buildaction.h>
#include <model/file.h>

namespace cc
{
namespace model
{

struct BuildAction;

#pragma db object
struct BuildSource
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FilePtr file;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;
};

typedef std::shared_ptr<BuildSource> BuildSourcePtr;

#pragma db object
struct BuildTarget
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FilePtr file;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;
};

typedef std::shared_ptr<BuildTarget> BuildTargetPtr;

} // model
} // cc

#endif // CC_MODEL_BUILDSOURCE_H
