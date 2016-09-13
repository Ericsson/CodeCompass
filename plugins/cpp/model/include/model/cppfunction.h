#ifndef CC_MODEL_CPPFUNCTION_H
#define CC_MODEL_CPPFUNCTION_H

#include <vector>

#include "cppentity.h"
#include "cppvariable.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppFunction : CppTypedEntity
{
  std::vector<odb::lazy_shared_ptr<CppVariable>> parameters;
  std::vector<odb::lazy_shared_ptr<CppVariable>> locals;

  bool isVirtual = false;
};

typedef std::shared_ptr<CppFunction> CppFunctionPtr;

}
}

#endif
