#ifndef CC_MODEL_CPPNAMESPACE_H
#define CC_MODEL_CPPNAMESPACE_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppNamespace : CppEntity
{

};

typedef std::shared_ptr<CppNamespace> CppNamespacePtr;

}
}

#endif
