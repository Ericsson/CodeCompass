#ifndef CC_MODEL_CPPTYPEDEF_H
#define CC_MODEL_CPPTYPEDEF_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppTypedef : CppTypedEntity
{

};

typedef std::shared_ptr<CppTypedef> CppTypedefPtr;

}
}

#endif
