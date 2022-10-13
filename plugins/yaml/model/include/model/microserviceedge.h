#ifndef CC_MODEL_YAMLEDGE_H
#define CC_MODEL_YAMLEDGE_H

#include <string>

#include <model/microservice.h>

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
  #pragma db on_delete(cascade)
  std::shared_ptr<Microservice> from;

  #pragma db not_null
  #pragma db on_delete(cascade)
  std::shared_ptr<Microservice> to;

  #pragma db not_null
  std::string type;

  std::string toString() const;
};

inline std::string MicroserviceEdge::toString() const
{
  return std::string("YamlEdge")
    .append("\nid = ").append(std::to_string(id))
    .append("\nfrom = ").append(std::to_string(from->id))
    .append("\nto = ").append(std::to_string(to->id))
    .append("\ntype = ");
}

inline std::uint64_t createIdentifier(const MicroserviceEdge& edge_)
{
  return util::fnvHash(
    std::to_string(edge_.from->id) +
    std::to_string(edge_.to->id) +
    edge_.type);
}

}
}

#endif // CC_MODEL_YAMLEDGE_H
