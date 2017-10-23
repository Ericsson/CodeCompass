#include "manglednamecache.h"

namespace cc
{
namespace parser
{

bool MangledNameCache::insert(const model::CppAstNode& node_)
{
  std::lock_guard<std::mutex> guard(_cacheMutex);
  return _mangledNameCache.insert(
    std::make_pair(node_.id, node_.mangledNameHash)).second;
}

std::uint64_t MangledNameCache::at(const model::CppAstNodeId& id_) const
{
  std::lock_guard<std::mutex> guard(_cacheMutex);
  return _mangledNameCache.at(id_);
}

void MangledNameCache::clear()
{
  _mangledNameCache.clear();
}

}
}
