#ifndef CC_MODEL_CPPENUM_H
#define CC_MODEL_CPPENUM_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppEnumConstant : CppEntity
{
  int value;
};

typedef std::shared_ptr<CppEnumConstant> CppEnumConstantPtr;

#pragma db object
struct CppEnum : CppEntity
{
  std::vector<odb::lazy_shared_ptr<CppEnumConstant>> enumConstants;
};

typedef std::shared_ptr<CppEnum> CppEnumPtr;

}
}

#endif
