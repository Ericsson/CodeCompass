#ifndef CC_MODEL_PYTHONVARIABLE_H
#define CC_MODEL_PYTHONVARIABLE_H

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonVariable : PythonEntity
{
    std::string toString() const
    {
        return std::string("PythonVariable")
                .append("\nid = ").append(std::to_string(id))
                .append("\nqualifiedName = ").append(qualifiedName);
    }
};

typedef std::shared_ptr <PythonVariable> PythonVariablePtr;

}
}

#endif
