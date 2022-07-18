#ifndef CC_MODEL_MICROSERVICE_H
#define CC_MODEL_MICROSERVICE_H

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct Microservice
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  std::string name;
};
}
}

#endif // CC_MODEL_MICROSERVICE_H
