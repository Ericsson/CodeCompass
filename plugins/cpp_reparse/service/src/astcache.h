#ifndef CC_SERVICE_CPPREPARSESERVICE_ASTCACHE_H
#define CC_SERVICE_CPPREPARSESERVICE_ASTCACHE_H

#include <chrono>
#include <map>
#include <memory>
#include <mutex>

// Required for the Thrift objects, such as core::FileId.
#include "cppreparse_types.h"

namespace clang
{
class ASTUnit;
} // namespace clang

namespace cc
{

namespace service
{

namespace reparse
{

// Because Clang's type hierarchy doesn't have a common parent (even with RTTI
// enabled), this very small value is used to indicate what type the memory
// address needs to be cast to.
enum class ClangASTNodeType : unsigned char
{
  Unknown,
  Decl,
  Stmt
};

/**
 * Implements a caching mechanism over Clang ASTs to ease the runtime overhead
 * of having to parse source files over and over again, as it is a *very*
 * costly operation.
 *
 * This class owns the ASTUnit instances cached. It is expected that this
 * class outlives the execution of FrontendActions over an AST.
 */
class ASTCache
{
public:
  typedef std::map<size_t, std::pair<void*, ClangASTNodeType>> NodeVisitMap;

  /**
   * @param maxCacheSize_ The maximum size of the cache above which automatic
   * pruning of old entries will take place. This is not an absolute limit, the
   * cache is allowed to overfill in case no more entries could be pruned.
   */
  ASTCache(size_t maxCacheSize_);

  ASTCache(const ASTCache&) = delete;
  ASTCache& operator=(const ASTCache&) = delete;
  ~ASTCache() = default;

  /**
   * Retrieves the AST stored for the given file ID, or a nullptr if none is
   * stored.
   */
  std::shared_ptr<clang::ASTUnit> getAST(const core::FileId& id_);

  /**
   * Store the AST for the given file in the cache. The AST Cache takes
   * ownership over the ASTUnit. The method returns the shared pointer that
   * wraps the ASTUnit argument given.
   */
  std::shared_ptr<clang::ASTUnit> storeAST(
    const core::FileId& id_,
    std::unique_ptr<clang::ASTUnit> AST_);

  /**
   * Retrieve the memory address and Clang AST node type stored for the given
   * file's node identified by visit order.
   *
   * The returned pair's memory address is nullptr if the file or the node is
   * not known by the cache.
   */
  NodeVisitMap::mapped_type getIdToAddressMapping(const core::FileId& id_,
                                                  size_t nodeVisitId_);

  /**
   * Stores a single node visit order ID -> Clang AST Node memory address
   * mapping for the given file's cached AST.
   */
   void storeIdToAddressMapping(const core::FileId& id_,
                                const size_t nodeVisitId_,
                                void* nodeAddress_,
                                const ClangASTNodeType type_);

private:

  class ASTCacheEntry
  {
  public:
    ASTCacheEntry(std::unique_ptr<clang::ASTUnit> AST_);
    ASTCacheEntry(const ASTCacheEntry&) = delete;
    ASTCacheEntry(ASTCacheEntry&&) = default;
    ~ASTCacheEntry() = default;
    ASTCacheEntry& operator=(const ASTCacheEntry&) = delete;
    ASTCacheEntry& operator=(ASTCacheEntry&&) = default;

    std::shared_ptr<clang::ASTUnit> getAST();

    size_t hitCount() const;
    std::chrono::steady_clock::time_point lastHit() const;

    /**
     * Returns the number of EXTERNAL (not counting the reference made by the
     * smart pointer stored in the current instance) references that are
     * present to the AST owned by the instance.
     */
    size_t referenceCount() const;

    void insertNodeMapping(const size_t nodeId_,
                           void* address_,
                           const ClangASTNodeType type_);
    NodeVisitMap& getNodeMap();

  private:
    std::shared_ptr<clang::ASTUnit> _AST;

    /**
     * The number of times the AST has been retrieved.
     */
    size_t _hitCount;

    /**
     * The last time the AST was retrieved from the cache. If the AST was never
     * retrieved, this timestamp stores the time when it was stored.
     */
    std::chrono::steady_clock::time_point _lastHit;

    /**
     * This inner cache maps node visit order numbers to the memory address
     * where the node is located.
     */
    // These mappings are only valid as long as the AST is cached, because a
    // new AST is always parsed with possibly different addresses of nodes.
    NodeVisitMap _nodeIdToAddress;
  };

  /**
   * Handles the clearing of the cache if there are more elements than
   * _maxCacheSize stored.
   */
  void pruneEntries();

  std::mutex _lock;
  std::map<core::FileId, ASTCacheEntry> _cache;
  size_t _maxCacheSize;
};

} // namespace reparse
} // namespace service
} // namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_ASTCACHE_H
