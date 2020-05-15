#ifndef CC_PARSER_COMPETENCEPARSER_H
#define CC_PARSER_COMPETENCEPARSER_H

#include <memory>

#include <boost/filesystem.hpp>
#include <odb/database.hxx>

#include <git2.h>

#include <model/file.h>
#include <parser/abstractparser.h>
#include <parser/parsercontext.h>
#include <util/parserutil.h>
#include <util/threadpool.h>

namespace cc
{
namespace parser
{

typedef std::unique_ptr<git_blame, decltype(&git_blame_free)> BlamePtr;
typedef std::unique_ptr<git_blame_options> BlameOptsPtr;
typedef std::unique_ptr<git_commit, decltype(&git_commit_free)> CommitPtr;
typedef std::unique_ptr<git_diff, decltype(&git_diff_free)> DiffPtr;
typedef std::unique_ptr<git_repository, decltype(&git_repository_free)> RepositoryPtr;
typedef std::unique_ptr<git_revwalk, decltype(&git_revwalk_free)> RevWalkPtr;
typedef std::unique_ptr<git_tree, decltype(&git_tree_free)> TreePtr;

typedef std::string UserEmail;
typedef int RelevantCommitCount;
typedef int Percentage;
typedef int UserBlameLines;

struct GitSignature
{
  std::string name;
  std::string email;
  std::uint64_t time;
};

struct GitBlameHunk
{
  std::uint32_t linesInHunk;        /**< The number of lines in this hunk. */
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

private:
  bool accept(const std::string& path_);

  std::shared_ptr<odb::database> _db;

  util::DirIterCallback getParserCallback(
    boost::filesystem::path& repoPath_);
  util::DirIterCallback getParserCallbackRepo(
    boost::filesystem::path& repoPath_);

  void loadCommitData(
    model::FilePtr file_,
    boost::filesystem::path& repoPath_);

  void persistEmailAddress(const std::string& email);

  void persistFileComprehensionData(
    model::FilePtr file_,
    const std::map<UserEmail, std::pair<Percentage, RelevantCommitCount>>& userEditions);

  RevWalkPtr createRevWalk(git_repository* repo_);

  BlamePtr createBlame(
    git_repository* repo_,
    const std::string& path_,
    git_blame_options* opts_);

  BlameOptsPtr createBlameOpts(const git_oid& newCommitOid_);

  RepositoryPtr createRepository(
    const boost::filesystem::path& repoPath_);

  CommitPtr createCommit(
    git_repository *repo_,
    const git_oid& id_);

  CommitPtr createParentCommit(
    git_commit* commit_);

  TreePtr createTree(
    git_commit* commit_);

  DiffPtr createDiffTree(
    git_repository* repo_,
    git_tree* first_,
    git_tree* second_);

  git_oid gitOidFromStr(const std::string& hexOid_);
  std::string gitOidToString(const git_oid* oid_);

  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;

  const int secondsInDay = 86400;
  const int daysInMonth = 30;

  int _maxCommitHistoryLength = 0;
  int _maxCommitCount = 0;
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_COMPETENCEPARSER_H
