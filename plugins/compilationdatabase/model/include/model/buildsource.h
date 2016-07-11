#ifndef CC_MODEL_BUILDSOURCE_H
#define CC_MODEL_BUILDSOURCE_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/common.h>
#include <model/buildaction.h>
#include <model/file.h>

namespace cc
{
namespace model
{

struct BuildSource;
struct BuildAction;
struct File;

typedef std::shared_ptr<BuildSource> BuildSourcePtr;

#pragma db object
struct BuildSource
{
  #pragma db id auto
  pktype id;

  #pragma db not_null
  FilePtr file;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;
};

} // model
} // cc

#endif // CC_MODEL_BUILDSOURCE_H
