#ifndef GITPARSER_GITPARSER_H
#define GITPARSER_GITPARSER_H

#include <model/file.h>
#include <parser/traversal.h>


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

class GitParser : public Traversal
{
public:
  GitParser(std::shared_ptr<model::Workspace> w_);

  ~GitParser();

public:
  virtual void beforeTraverse(
    const Traversal::OptionMap& opt_,
    SourceManager&) override;

  virtual DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_) override;
  
  virtual void endTraverse(const std::string&, SourceManager&) override;

private:
  
  std::shared_ptr<model::Workspace> _workspace;
  int _counter;
  std::string _projectDataDir;
};

} // parser
} // cc

#endif //GITPARSER_GITPARSER_H
