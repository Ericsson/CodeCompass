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

  std::string toString() const
  {
    return std::string("CppInheritance")
      .append("\nid = ").append(std::to_string(id))
      .append("\nderived = ").append(std::to_string(derived))
      .append("\nbase = ").append(std::to_string(base))
      .append("\nisVirtual = ").append(std::to_string(isVirtual))
      .append("\nvisibility = ").append(visibilityToString(visibility));
  }

#pragma db index member(derived)
#pragma db index member(base)
};

typedef std::shared_ptr<CppInheritance> CppInheritancePtr;

#pragma db view object(CppInheritance)
struct CppInheritanceCount
{
  #pragma db column("count(" + CppInheritance::id + ")")
  std::size_t count;
};

}
}

#endif
