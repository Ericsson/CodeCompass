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
    #pragma db id
    std::uint64_t id = 0;

    std::uint64_t ref_id;
    bool is_definition = false;
    bool is_builtin = false;
    std::string full_name;
    std::string value;
    std::string type;
    std::uint64_t line_start;
    std::uint64_t line_end;
    std::uint64_t column_start;
    std::uint64_t column_end;
    std::uint64_t file_id;
    std::string type_hint;
};

}
}

#endif