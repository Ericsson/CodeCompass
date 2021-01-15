#include "entitycache.h"

namespace cc
{
namespace parser
{

bool EntityCache::insert(const model::CppAstNode& node_)
{
  std::lock_guard<std::mutex> guard(_cacheMutex);
  return _entityCache.insert(
    std::make_pair(node_.id, node_.entityHash)).second;
}

std::uint64_t EntityCache::at(const model::CppAstNodeId& id_) const
{
  std::lock_guard<std::mutex> guard(_cacheMutex);
  return _entityCache.at(id_);
}

void EntityCache::clear()
{
  _entityCache.clear();
}

}
}
