#ifndef __CC_PARSER_METRICS_PARSER_H__
#define __CC_PARSER_METRICS_PARSER_H__

#include <parser/traversal.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/workspace.h>

#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{

class MetricsParser : public Traversal
{
public:
  MetricsParser(model::WorkspacePtr ws_);

  DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_) override;

private:
  struct Loc
  {
    unsigned originalLines;
    unsigned nonblankLines;
    unsigned codeLines;
  };
  
  Loc getLocFromFile(model::FilePtr file) const;
  
  void setCommentTypes(
    model::File::Type& type,
    std::string& singleComment,
    std::string& multiCommentStart,
    std::string& multiCommentEnd) const;
  
  void eraseBlankLines(std::string& file) const;
  
  void eraseComments(
    std::string& file,
    const std::string& singleComment,
    const std::string& multiCommentStart,
    const std::string& multiCommentEnd) const;
  
  void persistLoc(const Loc& loc, model::FileId file);
  
  odb::database* _db;
  util::OdbTransaction _transaction;
};

}
}

#endif