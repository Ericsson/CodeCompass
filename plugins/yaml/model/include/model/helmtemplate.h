#ifndef CC_MODEL_HELM_H
#define CC_MODEL_HELM_H

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>
#include <model/microservice.h>

namespace cc
{
namespace model
{
#pragma db object
struct HelmTemplate
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  std::string kind;

  #pragma db not_null
  MicroserviceId depends;
};
}
}

#endif // CC_MODEL_HELM_H
