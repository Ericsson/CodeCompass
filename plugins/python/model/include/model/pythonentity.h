#ifndef CC_MODEL_PYTHONENTITY_H
#define CC_MODEL_PYTHONENTITY_H

#include <string>

#include "pythonastnode.h"

#include <util/hash.h>

namespace cc
{
namespace model
{

typedef std::uint64_t PythonEntityId;

#pragma db object polymorphic
struct PythonEntity
{
    virtual ~PythonEntity() {}

    #pragma db id
    PythonEntityId id;

    #pragma db unique
    PythonAstNodeId astNodeId;

    std::string name;
    std::string qualifiedName;
    std::string visibility;
};

typedef std::shared_ptr <PythonEntity> PythonEntityPtr;

inline PythonEntityId createIdentifier(const PythonEntity& entity_)
{
    std::string res;

    res
        .append(std::to_string(entity_.astNodeId)).append(":")
        .append(entity_.name).append(":")
        .append(entity_.qualifiedName).append(":");

    return util::fnvHash(res);
}

}
}

#endif
