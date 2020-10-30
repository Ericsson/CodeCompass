#ifndef CC_MODEL_PYTHONENTITY_H
#define CC_MODEL_PYTHONENTITY_H

#include <string>

#include "pythonastnode.h"

namespace cc
{
namespace model
{

typedef std::uint64_t PythonEntityId;

#pragma db object polymorphic
struct PythonEntity
{
    virtual ~PythonEntity() {}

    #pragma db id auto
    PythonEntityId id;

    #pragma db unique
    PythonAstNodeId astNodeId;

    std::string name;
    std::string qualifiedName;
};

typedef std::shared_ptr <PythonEntity> PythonEntityPtr;

}
}

#endif
