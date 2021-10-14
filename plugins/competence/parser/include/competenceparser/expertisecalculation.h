#ifndef CC_PARSER_EXPERTISECALCULATION_H
#define CC_PARSER_EXPERTISECALCULATION_H

#include <memory>

#include <boost/filesystem.hpp>
//#include <odb/database.hxx>

#include <competenceparser/gitoperations.h>

#include <model/file.h>
#include <parser/abstractparser.h>
#include <parser/parsercontext.h>
#include <util/parserutil.h>
#include <util/threadpool.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

typedef std::string UserEmail;
typedef int RelevantCommitCount;
typedef double Percentage;
typedef int UserBlameLines;
typedef std::pair<Percentage, RelevantCommitCount> FileDataPair;

class ExpertiseCalculation
{
public:
  ExpertiseCalculation(ParserContext& ctx_);

  struct CommitJob
  {
    boost::filesystem::path& _repoPath;
    const std::string& _root;
    git_oid _oid;
    git_commit* _commit;
    int _commitCounter;

    CommitJob(boost::filesystem::path& repoPath_,
              const std::string& root_,
              git_oid oid_,
              git_commit* commit_,
              int commitCounter_)
      : _repoPath(repoPath_), _root(root_), _oid(oid_),
        _commit(commit_), _commitCounter(commitCounter_) {}
  };

  void initialize();

private:
  struct FileEdition
  {
    model::FilePtr _file;
    std::map<UserEmail, FileDataPair> _editions;
  };

  struct WalkData
  {
    //std::vector<const git_diff_delta*> deltas;
    const git_diff_delta* delta;
    git_repository* repo;
    boost::filesystem::path basePath;
    bool isParent = false;
    bool found = false;
  };

  struct WalkDeltaHunkData
  {
    const git_diff_delta* delta;
    const int commitNumber;
    git_repository* repo;
  };

  static int walkDeltaHunkCb(const char* root,
                             const git_tree_entry* entry,
                             void* payload);

  static int walkCb(const char* root,
                    const git_tree_entry* entry,
                    void* payload);
public:
  std::string plagiarismCommand(const std::string& extension);
  void commitWorker(CommitJob& job_);

  void traverseCommits(
    const std::string& root_,
    boost::filesystem::path& repoPath_);

  void persistEmailAddress();

  void persistCommitData(
    const model::FileId& fileId_,
    const std::map<UserEmail, UserBlameLines>& userBlame_,
    const float totalLines,
    git_status_t commitType_,
    const std::time_t& commitDate_);

  void collectFileLocData(CommitJob& job);

  void persistFileComprehensionData();
  util::DirIterCallback persistNoDataFiles();

  void setUserCompany();

  bool fileEditionContains(const std::string& path);

  // Temporary function to fill company list
  void setCompanyList();

  std::unique_ptr<util::JobQueueThreadPool<CommitJob>> _pool;

  std::vector<FileEdition> _fileEditions;
  std::set<UserEmail> _emailAddresses;
  std::map<std::string, std::string> _companyList;
  static std::map<std::pair<std::string, int>, int> _fileCommitLocData;
  std::map<std::string, std::vector<int>>  _fileLocData;

  std::mutex _calculateFileData;

  static GitOperations _gitOps;

  const int secondsInDay = 86400;
  const int daysInMonth = 30;
  const double plagThreshold = 100.0;

  int _maxCommitCount = 0;
  int _commitCount = 0;

  boost::filesystem::path basePath;

  ParserContext& _ctx;
};
}
}

#endif // CC_PARSER_EXPERTISECALCULATION_H
