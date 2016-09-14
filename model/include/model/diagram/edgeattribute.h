#ifndef MODEL_DIAGRAM_EDGEATTRIBUTE_H
#define	MODEL_DIAGRAM_EDGEATTRIBUTE_H

#include <memory>
#include <string>
#include "edge.h"

namespace cc
{
namespace model
{

#pragma db object
struct EdgeAttribute
{
  using pktype = int;

  #pragma db id auto
  pktype id;

  #pragma db not_null
  std::shared_ptr<Edge> edge;

  #pragma db not_null
  std::string key;

  #pragma db null
  std::string value;
};

#pragma db view object(EdgeAttribute)
struct EdgeAttributeIds
{
  EdgeAttribute::pktype id;
  Edge::pktype edge;
};

}
}

#endif
