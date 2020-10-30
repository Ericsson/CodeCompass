#ifndef CC_MODEL_PYTHONTYPE_H
#define CC_MODEL_PYTHONTYPE_H

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonType
{
    #pragma db id auto
    std::uint64_t id;

    PythonEntityId type;
    PythonEntityId symbol;
};

typedef std::shared_ptr<PythonType> PythonTypePtr;

}
}

#endif
