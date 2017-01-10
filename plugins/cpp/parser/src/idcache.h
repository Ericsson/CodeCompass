#ifndef CC_PARSER_IDCACHE_H
#define CC_PARSER_IDCACHE_H

#include <unordered_set>
#include <mutex>

namespace cc
{
namespace parser
{

/**
 * Thread safe id cache.
 */
class IdCache
{
public:
  /**
   * This function inserts an id to a cache in a thread-safe way.
   * @return If the insertion was successful (i.e. the cache didn't contain the
   * id before) then the function returns true.
   */
  bool insert(std::uint64_t id_);

  /**
   * Removes all elements from the cache.
   */
  void clear();

private:
  std::unordered_set<std::uint64_t> _idCache;
  std::mutex _cacheMutex;
};

} // parser
} // cc

#endif // CC_PARSER_IDCACHE_H
