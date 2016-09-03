#ifndef CC_MODEL_CPPINHERITANCE_H
#define CC_MODEL_CPPINHERITANCE_H

#include <memory>
#include "common.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppInheritance
{
  #pragma db id auto
  int id;

  std::uint64_t derived;
  std::uint64_t base;

  bool isVirtual = false;
  Visibility visibility;

#ifndef NO_INDICES
  #pragma db index member(derived)
  #pragma db index member(base)
#endif
};

typedef std::shared_ptr<CppInheritance> CppInheritancePtr;

}
}

#endif
