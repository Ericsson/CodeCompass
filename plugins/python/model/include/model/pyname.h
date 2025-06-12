#ifndef CC_MODEL_PYNAME_H
#define CC_MODEL_PYNAME_H

#include <cstdint>
#include <string>
#include <odb/core.hxx>

namespace cc
{
namespace model
{

enum PYNameID {
  ID,
  REF_ID,
  PARENT,
  PARENT_FUNCTION
};

#pragma db object
struct PYName
{
    #pragma db id unique
    std::uint64_t id = 0;

    #pragma db index
    std::uint64_t ref_id;

    std::uint64_t parent;
    std::uint64_t parent_function;

    bool is_definition = false;
    bool is_builtin = false;
    bool is_import = false;
    bool is_call = false;
    std::string full_name;
    std::string value;
    std::string type;
    std::string type_hint;

    std::uint64_t line_start;
    std::uint64_t line_end;
    std::uint64_t column_start;
    std::uint64_t column_end;

    #pragma db index
    std::uint64_t file_id;
};

}
}

#endif
