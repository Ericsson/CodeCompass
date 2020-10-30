#ifndef CC_MODEL_PYTHONFUNCTION_H
#define CC_MODEL_PYTHONFUNCTION_H

#include <odb/lazy-ptr.hxx>

#include "pythonvariable.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonFunction : PythonEntity
{
    #pragma db on_delete(cascade)
    std::vector<odb::lazy_shared_ptr<PythonVariable>> parameters;

    #pragma db on_delete(cascade)
    std::vector<odb::lazy_shared_ptr<PythonVariable>> locals;

    std::string toString() const
    {
        return std::string("PythonFunction")
                .append("\nid = ").append(std::to_string(id))
                .append("\nqualifiedName = ").append(qualifiedName);
    }
};

typedef std::shared_ptr<PythonFunction> PythonFunctionPtr;

#pragma db view \
  object(PythonFunction) object(PythonVariable = Parameters : PythonFunction::parameters)
struct PythonFunctionParamCount
{
    #pragma db column("count(" + Parameters::id + ")")
    std::size_t count;
};

#pragma db view \
  object(PythonFunction) object(PythonVariable = Locals : PythonFunction::locals)
struct PythonFunctionLocalCount
{
    #pragma db column("count(" + Locals::id + ")")
    std::size_t count;
};

}
}

#endif
