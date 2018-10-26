#include <algorithm>

#include <clang/Frontend/ASTUnit.h>

#include <util/logutil.h>

#include "astcache.h"

namespace cc
{

namespace service
{

namespace reparse
{

using namespace clang;

ASTCache::ASTCache(size_t maxCacheSize_)
  : _maxCacheSize(maxCacheSize_)
{}

std::shared_ptr<clang::ASTUnit> ASTCache::getAST(const core::FileId& id_)
{
  std::lock_guard<std::mutex> lock(_lock);
  auto it = _cache.find(id_);
  if (it == _cache.end())
    return nullptr;

  return it->second.getAST();
}

std::shared_ptr<ASTUnit> ASTCache::storeAST(
  const core::FileId& id_,
  std::unique_ptr<ASTUnit> AST_)
{
  if (_cache.size() >= _maxCacheSize)
    pruneEntries();

  std::lock_guard<std::mutex> lock(_lock);
  auto it = _cache.find(id_);
  if (it != _cache.end())
    // If the key already exists in the map, it has to be overwritten.
    // This cannot be done pre-C++17 without clearing the element first.
    _cache.erase(it);

  auto result = _cache.emplace(id_, std::move(AST_));
  return result.first->second.getAST();
}

void ASTCache::pruneEntries()
{
  std::lock_guard<std::mutex> lock(_lock);

  while (_cache.size() >= _maxCacheSize)
  {
    // For now, remove the element who wasn't touched for the longest time.
    auto elemToRemove = std::min_element(
        _cache.begin(), _cache.end(),
        [](const auto& lhs, const auto& rhs)
        {
          return lhs.second.lastHit() < rhs.second.lastHit();
        });

    _cache.erase(elemToRemove);
  }
}

ASTCache::ASTCacheEntry::ASTCacheEntry(std::unique_ptr<clang::ASTUnit> AST_)
  : _AST(std::move(AST_)),
    _hitCount(0),
    _lastHit(std::chrono::steady_clock::now())
{}

std::shared_ptr<ASTUnit> ASTCache::ASTCacheEntry::getAST()
{
  ++_hitCount;
  _lastHit = std::chrono::steady_clock::now();
  return _AST;
}

size_t ASTCache::ASTCacheEntry::hitCount() const
{
  return _hitCount;
}

std::chrono::steady_clock::time_point ASTCache::ASTCacheEntry::lastHit() const
{
  return _lastHit;
}

size_t ASTCache::ASTCacheEntry::referenceCount() const
{
  auto useCount = static_cast<size_t>(_AST.use_count());
  assert(useCount > 0 && "Use count must not reach zero in the reference "
                         "object if it is still in the cache.");
  return useCount - 1;
}

} // namespace language
} // namespace service
} // namespace cc
