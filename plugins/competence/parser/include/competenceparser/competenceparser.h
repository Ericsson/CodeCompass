#ifndef CC_PARSER_COMPETENCEPARSER_H
#define CC_PARSER_COMPETENCEPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <memory>

#include <git2.h>

#include <model/file.h>
#include <odb/database.hxx>


namespace cc
{
namespace parser
{

typedef std::unique_ptr<git_blame, decltype(&git_blame_free)> BlamePtr;
typedef std::unique_ptr<git_blame_options> BlameOptsPtr;
typedef std::unique_ptr<git_commit, decltype(&git_commit_free)> CommitPtr;
typedef std::unique_ptr<git_repository, decltype(&git_repository_free)> RepositoryPtr;

struct GitSignature
{
  std::string name;
  std::string email;
  std::uint64_t time;
};

struct GitBlameHunk
{
  std::uint32_t linesInHunk;             /**< The number of lines in this hunk. */
  std::string finalCommitId;        /**< The OID of the commit where this line was
                                    last changed. */
  std::string finalCommitMessage;   /**< Commit message of the commit specified by
                                    finalCommitId. */
  std::uint32_t finalStartLineNumber;    /**< The 1-based line number where this hunk
                                    begins, in the final version of the file.
                               */
  GitSignature finalSignature; /**< Signature of the commit who this line
                                    last changed. */
  std::string origCommitId;         /**< The OID of the commit where this hunk was
                                    found. This will usually be the same as
                                    final_commit_id, except when
                                    GIT_BLAME_TRACK_COPIES_ANY_COMMIT_COPIES
                                    has been specified.
                                    TODO: implement blame flags. */
  std::string origPath;             /**< The path to the file where this hunk
                                    originated, as of the commit specified by
                                    origCommitId. */
  std::uint32_t origStartLineNumber;     /**< The 1-based line number where this hunk
                                    begins in the file named by orig_path in
                                    the commit specified by origCommitId. */
  GitSignature origSignature;  /**< Signature of the commit who thisline last
                                    changed. */
  bool boundary;              /**< True if the hunk has been tracked to a
                                    boundary commit (the root, or the commit
                                    specified in
                                    git_blame_options.oldest_commit). */
};
  
class CompetenceParser : public AbstractParser
{
public:
  CompetenceParser(ParserContext& ctx_);
  virtual ~CompetenceParser();
  virtual bool parse() override;

  void loadRepositoryData(std::string& return_,
    const model::FileId& fileId_,
    const std::string& repoId_,
    const std::string& hexOid_,
    const std::string& path_,
    const std::string& user_ = "afekete");
private:
  bool accept(const std::string& path_);

  std::shared_ptr<odb::database> _db;
  std::shared_ptr<std::string> _datadir;

  BlamePtr createBlame(
    git_repository* repo_,
    const std::string& path_,
    git_blame_options* opts_);
  BlameOptsPtr createBlameOpts(const git_oid& newCommitOid_);
  RepositoryPtr createRepository(const std::string& repoId_);
  CommitPtr createCommit(git_repository *repo_,
                         const git_oid& id_);
  git_oid gitOidFromStr(const std::string& hexOid_);
  std::string gitOidToString(const git_oid* oid_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_COMPETENCEPARSER_H
