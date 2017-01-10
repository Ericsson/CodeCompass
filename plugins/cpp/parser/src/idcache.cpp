#include "idcache.h"

namespace cc
{
namespace parser
{

bool IdCache::insert(std::uint64_t id_)
{
  std::lock_guard<std::mutex> guard(_cacheMutex);
  return _idCache.insert(id_).second;
}

void IdCache::clear()
{
  _idCache.clear();
}

}
}
