#ifndef CC_MODEL_PYNAME_H
#define CC_MODEL_PYNAME_H

#include <cstdint>
#include <string>
#include <odb/core.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct PYName
{
    enum class PYNameType
    {
        Module, Class, Instance, Function, Param, Path, Keyword, Property, Statement, Unknown
    };

    #pragma db id
    std::uint64_t id = 0;

    std::uint64_t refid;
    bool is_definition = false;
    bool is_builtin = false;
    std::string full_name;
    PYNameType type;
    std::uint64_t line;
    std::uint64_t column;
    std::uint64_t line_start;
    std::uint64_t line_end;
    std::uint64_t column_start;
    std::uint64_t column_end;
};
}
}

#endif