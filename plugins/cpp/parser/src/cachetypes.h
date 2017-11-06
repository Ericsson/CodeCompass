#ifndef CC_PARSER_CACHETYPES_H
#define CC_PARSER_CACHETYPES_H

#include <map>
#include <set>

#include <model/cppastnode.h>
#include <util/threadsafecontainer.h>

namespace cc
{
namespace parser
{

using IdCache = util::ThreadSafeSet<std::unordered_set<std::uint64_t>>;

using MangledNameCache
  = util::ThreadSafeMap<std::unordered_map<model::CppAstNodeId, std::uint64_t>>;

}
}

#endif
