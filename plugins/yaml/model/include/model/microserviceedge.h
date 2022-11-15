#ifndef CC_MODEL_YAMLEDGE_H
#define CC_MODEL_YAMLEDGE_H

#include <string>

#include <model/microservice.h>
#include "model/helmtemplate.h"

#include <util/hash.h>

namespace cc
{
namespace model
{

struct MicroserviceEdge;
typedef std::shared_ptr<MicroserviceEdge> MicroserviceEdgePtr;

typedef std::uint64_t MicroserviceEdgeId;

#pragma db object
struct MicroserviceEdge
{
  #pragma db id
  MicroserviceEdgeId id;

  #pragma db not_null
  uint64_t helperId;

  #pragma db not_null
  #pragma db on_delete(cascade)
  std::shared_ptr<Microservice> from;

  #pragma db not_null
  #pragma db on_delete(cascade)
  std::shared_ptr<Microservice> to;

  #pragma db not_null
  #pragma db on_delete(cascade)
  std::shared_ptr<HelmTemplate> connection;

  #pragma db not_null
  std::string type;

  std::string toString() const;
};

inline std::string MicroserviceEdge::toString() const
{
  return std::string("MicroserviceEdge")
    //.append("\nid = ").append(std::to_string(id))
    .append("\nfrom = ").append(std::to_string(from->serviceId))
    .append("\nto = ").append(std::to_string(to->serviceId))
    .append("\ntype = ");
}

inline std::uint64_t createIdentifier(const MicroserviceEdge& edge_)
{
  return util::fnvHash(
    std::to_string(edge_.from->serviceId) +
    std::to_string(edge_.to->serviceId) +
    std::to_string(edge_.helperId) +
    edge_.type);
}

}
}

#endif // CC_MODEL_YAMLEDGE_H
