#ifndef CC_MODEL_BUILDDIRECTORY_H
#define CC_MODEL_BUILDDIRECTORY_H

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
struct BuildDirectory
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  std::string directory;

  #pragma db not_null
  #pragma db on_delete(cascade)
  std::shared_ptr<BuildAction> action;
};

typedef std::shared_ptr<BuildDirectory> BuildDirectoryPtr;

} // model
} // cc

#endif // CC_MODEL_BUILDDIRECTORY_H
