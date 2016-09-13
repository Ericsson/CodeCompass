#ifndef CC_MODEL_CPPVARIABLE_H
#define CC_MODEL_CPPVARIABLE_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppVariable : CppTypedEntity
{
  bool isGlobal = false;
};

typedef std::shared_ptr<CppVariable> CppVariablePtr;

}
}

#endif
