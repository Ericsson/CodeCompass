#ifndef CC_MODEL_MICROSERVICE_H
#define CC_MODEL_MICROSERVICE_H

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include "model/file.h"

#include "util/hash.h"

namespace cc
{
namespace model
{

typedef std::uint64_t MicroserviceId;

#pragma db object
struct Microservice
{
  enum class ServiceType
  {
    INTERNAL,
    EXTERNAL
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  MicroserviceId serviceId;

  #pragma db not_null
  std::string name;

  //#pragma db
  FileId file;

  //#pragma db
  ServiceType type;
};

inline std::uint64_t createIdentifier(const Microservice& service_)
{
  return util::fnvHash(
    service_.name);
}
}
}

#endif // CC_MODEL_MICROSERVICE_H
