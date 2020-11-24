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

    std::string toString() const
    {
        return std::string("PythonType")
                .append("\nid = ").append(std::to_string(id))
                .append("\ntype = ").append(std::to_string(type))
                .append("\nsymbol = ").append(std::to_string(symbol));
    }
};

typedef std::shared_ptr<PythonType> PythonTypePtr;

}
}

#endif
