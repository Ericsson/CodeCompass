#ifndef SEARCHPARSER_SEARCHPARSER_H
#define SEARCHPARSER_SEARCHPARSER_H

#include <model/file.h>
#include <parser/traversal.h>

#include <util/threadpool.h>

#include <magic.h>

namespace clang
{
  class ASTUnit;
}

namespace cc
{

namespace model
{
  class Workspace;
}

namespace parser
{

class IndexerProcess;
class SourceManager;

class SearchParser : public Traversal
{
public:
  SearchParser(std::shared_ptr<model::Workspace> w_);

  ~SearchParser();

public:
  virtual void beforeTraverse(
    const Traversal::OptionMap& projectOptions_,
    SourceManager&) override;

  virtual void afterTraverse(SourceManager&) override;

  virtual DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_) override;

private:
  /**
   * @param path_ file path to check
   * @return true if the file should be handled
   */
  bool shouldHandle(const std::string& path_);

  /**
   * The database workspace object.
   */
  std::shared_ptr<model::Workspace> _workspace;

  /**
   * Java index process.
   */
  std::unique_ptr<IndexerProcess> _indexProcess;
  
  /**
   * libmagic handler for mime types.
   */
  ::magic_t _fileMagic;
};

} // parser
} // cc

#endif //SEARCHPARSER_SEARCHPARSER_H
