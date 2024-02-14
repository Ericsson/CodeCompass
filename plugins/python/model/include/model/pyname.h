#ifndef CC_MODEL_PYNAME_H
#define CC_MODEL_PYNAME_H

#include <cstdint>
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

    std::uint64_t line;
    std::uint64_t column;
    std::uint64_t refID;
};
}
}

#endif