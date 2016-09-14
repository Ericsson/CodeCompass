#ifndef CODECOMPASS_MODEL_BUILD_TARGET_H
#define CODECOMPASS_MODEL_BUILD_TARGET_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/buildaction.h>
#include <model/file.h>

namespace cc
{
namespace model
{

struct BuildTarget;
struct BuildAction;
struct File;

typedef std::shared_ptr<BuildTarget> BuildTargetPtr;

#pragma db object
struct BuildTarget
{
  #pragma db id auto
  unsigned long id;

  #pragma db not_null
  FilePtr file;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_BUILD_TARGET_H
