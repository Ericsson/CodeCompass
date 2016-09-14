#ifndef MODEL_DIAGRAM_NODEATTRIBUTE_H
#define	MODEL_DIAGRAM_NODEATTRIBUTE_H

#include <memory>
#include <string>
#include "node.h"

namespace cc
{
namespace model
{

#pragma db object
class NodeAttribute
{
public:
  #pragma db id auto
  int id;
  
  #pragma db not_null
  std::shared_ptr<Node> node;
  
  #pragma db not_null
  std::string key;
  
  #pragma db null
  std::string value;
};

}
}

#endif