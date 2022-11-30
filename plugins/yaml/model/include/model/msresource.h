#ifndef CC_MODEL_MSRESOURCE_H
#define CC_MODEL_MSRESOURCE_H

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include "model/file.h"
#include "microservice.h"

#include "util/hash.h"

namespace cc
{
namespace model
{

#pragma db object
struct MSResource
{
  enum class ResourceType
  {
    CPU,
    MEMORY,
    STORAGE
  };

  #pragma db id auto
  uint64_t id;

  #pragma db not_null
  ResourceType type;

  #pragma db not_null
  MicroserviceId service;

  #pragma db not_null
  float amount;

  #pragma db not_null
  std::string unit;
};
}
}

#endif // CC_MODEL_MSRESOURCE_H
