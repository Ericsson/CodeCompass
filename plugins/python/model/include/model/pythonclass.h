#ifndef CC_MODEL_PYTHONCLASS_H
#define CC_MODEL_PYTHONCLASS_H

#include "pythonentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct PythonClass : PythonEntity
{
    std::string toString() const
    {
        return std::string("PythonClass")
                .append("\nid = ").append(std::to_string(id))
                .append("\nqualifiedName = ").append(qualifiedName);
    }
};

typedef std::shared_ptr<PythonClass> PythonClassPtr;

#pragma db object
struct PythonClassMember
{
    enum Kind
    {
        Attribute,
        Method,
        Class
    };

    #pragma db id auto
    int id;

    #pragma db unique
    PythonAstNodeId astNodeId;

    PythonEntityId memberId;
    PythonEntityId classId;

    Kind kind;
    bool staticMember = false;

    std::string toString() const
    {
        return std::string("PythonClassMember")
                .append("\nid = ").append(std::to_string(id))
                .append("\nmemberId = ").append(std::to_string(memberId))
                .append("\nclassId = ").append(std::to_string(classId))
                .append("\nstaticMember = ").append(std::to_string(staticMember))
                .append("\nkind = ").append(kind == Attribute ? "Attribute" :
                            kind == Method ? "Method" : "Class");
    }
};

typedef std::shared_ptr<PythonClassMember> PythonClassMemberPtr;

#pragma db view object(PythonClass)
struct PythonClassCount
{
    #pragma db column("count(" + PythonClass::id + ")")
    std::size_t count;
};

#pragma db view object(PythonClassMember)
struct PythonClassMemberCount
{
    #pragma db column("count(" + PythonClassMember::id + ")")
    std::size_t count;
};

}
}

#endif
