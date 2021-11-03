#ifndef CC_MODEL_PYTHONINHERITANCE_H
#define CC_MODEL_PYTHONINHERITANCE_H

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonInheritance
{
    #pragma db id auto
    int id;

    PythonEntityId derived;
    PythonEntityId base;

    std::string toString() const
    {
        return std::string("PythonInheritance")
            .append("\nid = ").append(std::to_string(id))
            .append("\nderived = ").append(std::to_string(derived))
            .append("\nbase = ").append(std::to_string(base));
    }

#pragma db index member(derived)
#pragma db index member(base)
};

typedef std::shared_ptr<PythonInheritance> PythonInheritancePtr;

#pragma db view object(PythonInheritance)
struct PythonInheritanceCount
{
    #pragma db column("count(" + PythonInheritance::id + ")")
    std::size_t count;
};

}
}

#endif
