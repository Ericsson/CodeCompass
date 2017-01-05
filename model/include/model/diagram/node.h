#ifndef NODE_H
#define	NODE_H

#include <string>

namespace cc
{
namespace model
{

#pragma db object
struct Node
{
  using pktype = int;

  enum Domain
  {
    FILE,
    CPPASTNODE,
    MI
  };

  #pragma db id auto
  pktype id;

  #pragma db not_null
  Domain domain;

  #pragma db not_null
  std::string domainId;
};

}
}

#endif	/* NODE_H */
