#ifndef PARSER_INTERNAL_TRAVERSAL_H
#define PARSER_INTERNAL_TRAVERSAL_H

#include <string>
#include <map>
#include <functional>

namespace cc 
{ 
namespace parser
{

class SourceManager;

/**
 * Interface for traversal parsers. Its a callback based parser interface.
 *
 * The callback methods are called by the following logic (pseudo code):
 *
 * ----------------------------------
 * beforeTraverse(...)
 * for root in roots:
 *   cb = traverse(..., root, ...)
 *   if cb is not empty:
 *     for file in files under root:
 *       cb(..., file, ...)
 *     endTraverse(..., root, ...)
 * afterTraverse(...)
 * -----------------------------------
 *
 * Note: The endTraverse method is only called when traverse() returned a
 *       non-empty callback (see the pseudo code).
 */
class Traversal
{
public:
  /**
   * Enum for file types.
   */
  enum class FileType
  {
    Other,
    Directory,
    RegularFile
  };
  
  virtual ~Traversal();

  /**
   * Callback function type for iterateDirectoryRecursive.
   *
   * The first parameter is the full path of the current entity.
   * The second parameter is the type of the current entity.
   *
   * If the callback returns false, then the iteration stops.
   */
  typedef std::function<bool (const std::string&, FileType)> DirIterCallback;
  
  /**
   * Type alias for configuration map.
   */
  using OptionMap = std::map<std::string, std::string>;

  /**
   * Called before any traverse and endTraverse call. Its a good place to do
   * some initialization.
   *
   * @param projectOptions_ project parameters.
   * @param srcMgr_ a source manager.
   */
  virtual void beforeTraverse(
    const OptionMap& projectOptions_,
    SourceManager& srcMgr_);

  /**
   * Called after traverse and endTraverse calls. It is guaranteed that no more
   * traverse or endTraverse call will happen on this object without a
   * beforeTraverse call.
   *
   * @param srcMgr_ a source manager.
   */
  virtual void afterTraverse(SourceManager& srcMgr_);

  /**
   * This method should return a function object that will be called for all
   * entry under the given directory (path_). If a traverse returns an empty
   * function object than it will be ignored.
   * 
   * The default implementation just returns an empty function.
   *
   * @param projectOptions_ project parameters.
   * @param path_ path that will be iterated.
   * @param srcMgr_ a source manager.
   * @return a function object.
   */
  virtual DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_);
  
  /**
   * Called after the last call to the callback returned by traverse method.
   * Its a good place to do commits to a 3rd party database. It is called even
   * if the traverse method returned an empty object.
   * 
   * @param path_ the that was iterated.
   * @param srcMgr_ a source manager.
   */
  virtual void endTraverse(
    const std::string& path_,
    SourceManager& srcMgr_);

protected:
  std::size_t _numOfSuccess = 0;
  std::size_t _numOfFail = 0;
};

} //parser
} //cc

#endif 
