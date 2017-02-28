#ifndef CC_PARSER_MANGLEDNAMECACHE_H
#define CC_PARSER_MANGLEDNAMECACHE_H

#include <unordered_map>
#include <mutex>

#include <model/cppastnode.h>

namespace cc
{
namespace parser
{

/**
 * Thread safe mangled name cache.
 */
class MangledNameCache
{
public:
  /**
   * This function inserts a model::CppAstNodeId to a cache in a
   * thread-safe way.
   * @return If the insertion was successful (i.e. the cache didn't contain the
   * id before) then the function returns true.
   */
  bool insert(const model::CppAstNode& node_);

  /**
   * Returns a reference to the mapped value of the element with key equivalent
   * to id_. If no such element exists, an exception of type
   * std::out_of_range is thrown.
   */
  std::uint64_t at(const model::CppAstNodeId& id_) const;

  /**
   * Removes all elements from the cache.
   */
  void clear();

private:
  std::unordered_map<model::CppAstNodeId, std::uint64_t> _mangledNameCache;
  std::mutex _cacheMutex;
};

} // parser
} // cc

#endif // CC_PARSER_MANGLEDNAMECACHE_H
