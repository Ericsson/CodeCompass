#ifndef CC_MODEL_PYTHONIMPORT_H
#define CC_MODEL_PYTHONIMPORT_H

#include <odb/lazy-ptr.hxx>

#include <model/file.h>

#include <model/pythonastnode.h>

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonImport
{
    #pragma db id auto
    int id;

    #pragma db unique
    PythonAstNodeId astNodeId;

    #pragma db not_null
    #pragma db on_delete(cascade)
    odb::lazy_shared_ptr<File> importer;

    #pragma db not_null
    #pragma db on_delete(cascade)
    odb::lazy_shared_ptr<File> imported;

    PythonEntityId importedSymbol;

    std::string toString() const
    {
        return std::string("PythonModuleImport")
            .append("\nid = ").append(std::to_string(id))
            .append("\nimporter = ").append(std::to_string(importer->id))
            .append("\nimported = ").append(std::to_string(imported->id))
            .append("\nimported symbols =").append(std::to_string(importedSymbol));
    }

#pragma db index member(importer)
#pragma db index member(imported)
};

typedef std::shared_ptr<PythonImport> PythonImportPtr;

#pragma db view object(PythonImport)
struct PythonImportCount
{
    #pragma db column("count(" + PythonImport::id + ")")
    std::size_t count;
};

}
}

#endif
