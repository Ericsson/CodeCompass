#ifndef CC_MODULE_PYTHONDOCUMENTATION_H
#define CC_MODULE_PYTHONDOCUMENTATION_H

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonDocumentation
{
    enum Kind
    {
        Function,
        Class,
        Module
    };

    #pragma db id auto
    int id;

    std::string documentation;

    PythonEntityId documented;

    Kind documentationKind;

    std::string toString() const
    {
        return std::string("PythonPythonDocumentation")
                .append("\nid = ").append(std::to_string(id))
                .append("\ndocumentation = ").append(documentation)
                .append("\ndocumented = ").append(std::to_string(documented))
                .append("\ndocumentationKind = ").append(
                        documentationKind == Function ? "Function" :
                        documentationKind == Class ? "Class" : "Module");
    }
};

typedef std::shared_ptr<PythonDocumentation> PythonDocumentationPtr;

}
}

#endif
