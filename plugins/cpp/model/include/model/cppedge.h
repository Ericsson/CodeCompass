#ifndef CC_MODEL_CPPEDGE_H
#define CC_MODEL_CPPEDGE_H

#include <memory>
#include <string>
#include <tuple>

#include <util/hash.h>

#include "cppnode.h"

namespace cc
{
namespace model
{

typedef std::uint64_t CppEdgeId;

#pragma db object
struct CppEdge
{
  /**
   * This enum has to be extended if a new type needed. It has the advantage
   * over a simple string that this way it can be ensured by convention that two
   * different modules don't use the same edge type.
   */
  enum Type {
    PROVIDE,
    USE
  };

  #pragma db id
  CppEdgeId id;

  #pragma db not_null
  std::shared_ptr<File> from;

  #pragma db not_null
  std::shared_ptr<File> to;

  #pragma db not_null
  Type type;

  std::string toString() const;

  bool operator<(const CppEdge& other) const
  {
    return std::make_tuple(from->id, to->id, type)
         < std::make_tuple(other.from->id, other.to->id, other.type);
  }
};

typedef std::shared_ptr<CppEdge> CppEdgePtr;

inline std::string typeToString(CppEdge::Type type_)
{
  switch (type_)
  {
    case CppEdge::Type::PROVIDE: return "Provide";
    case CppEdge::Type::USE: return "Use";
  }

  return std::string();
}

inline std::string CppEdge::toString() const
{
  return std::string("CppEdge")
    .append("\nid = ").append(std::to_string(id))
    .append("\nfrom = ").append(std::to_string(from->id))
    .append("\nto = ").append(std::to_string(to->id))
    .append("\ntype = ").append(typeToString(type));
}

inline std::uint64_t createIdentifier(const CppEdge& edge_)
{
  return util::fnvHash(
    std::to_string(edge_.from->id) +
    std::to_string(edge_.to->id) +
    typeToString(edge_.type));
}

typedef std::uint64_t CppEdgeAttributeId;

#pragma db object
struct CppEdgeAttribute
{
  #pragma db id
  CppEdgeAttributeId id;

  #pragma db not_null
  std::shared_ptr<CppEdge> edge;

  #pragma db not_null
  std::string key;

  #pragma db null
  std::string value;

  std::string toString() const;
};

typedef std::shared_ptr<CppEdgeAttribute> CppEdgeAttributePtr;

inline std::string CppEdgeAttribute::toString() const
{
  return std::string("CppEdge")
    .append("\nid = ").append(std::to_string(id))
    .append("\nkey = ").append(key)
    .append("\nvalue = ").append(value);
}

inline std::uint64_t createIdentifier(const CppEdgeAttribute& attr_)
{
  return util::fnvHash(
    std::to_string(attr_.edge->id) + attr_.key + attr_.value);
}

} // model
} // cc

#endif // CC_MODEL_CPPEDGE_H
