#ifndef MODEL_DIAGRAM_EDGE_H
#define	MODEL_DIAGRAM_EDGE_H

#include <memory>
#include <string>
#include <tuple>
#include "node.h"

namespace cc
{
namespace model
{

#pragma db object
struct Edge
{
  using pktype = int;

  /**
   * This enum has to be extended if a new type needed. It has the advantage
   * over a simple string that this way it can be ensured by convention that two
   * different modules don't use the same edge type.
   */
  enum Type {
    PROVIDE,
    IMPLEMENT,
    USE,
    DEPEND
  };

  #pragma db id auto
  pktype id;

  #pragma db not_null
  std::shared_ptr<Node> from;

  #pragma db not_null
  std::shared_ptr<Node> to;

  #pragma db not_null
  Type type;

  bool operator<(const Edge& other) const
  {
    return std::make_tuple(from->id, to->id, type)
         < std::make_tuple(other.from->id, other.to->id, other.type);
  }
};

#pragma db view object(Edge)
struct EdgeIds
{
  Edge::pktype id;
  Node::pktype from;
  Node::pktype to;
};

}
}

#endif
