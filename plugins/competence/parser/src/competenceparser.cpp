#include <competenceparser/competenceparser.h>

#include <chrono>
#include <memory>
#include <cctype>

#include <model/commitdata.h>
#include <model/commitdata-odb.hxx>
#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/sampledata.h>
#include <model/sampledata-odb.hxx>
#include <model/useremail.h>
#include <model/useremail-odb.hxx>

#include <parser/sourcemanager.h>

#include <util/hash.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{

CompetenceParser::CompetenceParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  git_libgit2_init();

  srand(time(nullptr));

  int threadNum = _ctx.options["jobs"].as<int>();
  _pool = util::make_thread_pool<CommitJob>(
    threadNum, [this](CommitJob& job)
    {
      this->commitWorker(job);
    });

  setCompanyList();

  if (_ctx.options.count("commit-count"))
  {
    _maxCommitCount = _ctx.options["commit-count"].as<int>();

    if (_maxCommitCount < 1)
    {
      LOG(fatal) << "Commit count is too small!";
      throw std::logic_error("");
    }
    LOG(info) << "[competenceparser] Commit history of " << _maxCommitCount << " commits will be parsed.";
    return;
  }
}

bool CompetenceParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".competence";
}

bool CompetenceParser::parse()
{        
  for(const std::string& path :
    _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Competence parse path: " << path;

    boost::filesystem::path repoPath;

    auto rcb = getParserCallbackRepo(repoPath);

    try
    {
      util::iterateDirectoryRecursive(path, rcb);
    }
    catch (std::exception &ex_)
    {
      LOG(warning)
        << "Competence parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      LOG(warning)
        << "Competence parser failed with unknown exception!";
    }

    std::string repoId = std::to_string(util::fnvHash(repoPath.c_str()));
    RepositoryPtr repo = createRepository(repoPath);

    if (!repo)
      continue;

    util::OdbTransaction transaction(_ctx.db);
    transaction([&, this]
    {
      for (const model::FileComprehension& fc
        : _ctx.db->query<model::FileComprehension>())
        _ctx.db->erase(fc);

      for (const model::UserEmail& ue
        : _ctx.db->query<model::UserEmail>())
        _ctx.db->erase(ue);
    });

    commitSampling(path, repoPath);
    traverseCommits(path, repoPath);
    persistFileComprehensionData();

    auto pcb = persistNoDataFiles();

    try
    {
      util::iterateDirectoryRecursive(path, pcb);
    }
    catch (std::exception &ex_)
    {
      LOG(warning)
        << "Competence parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      LOG(warning)
        << "Competence parser failed with unknown exception!";
    }

    persistEmailAddress();
    setUserCompany();
  }

  return true;
}

util::DirIterCallback CompetenceParser::getParserCallbackRepo(
  boost::filesystem::path& repoPath_)
{
  return [&](const std::string& path_)
  {
    boost::filesystem::path path(path_);

    if (!boost::filesystem::is_directory(path) || ".git" != path.filename())
      return true;

    path = boost::filesystem::canonical(path);

    LOG(info) << "Competence parser found a git repo at: " << path;
    repoPath_ = path_;

    return true;
  };
}

void CompetenceParser::commitSampling(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = createRepository(repoPath_);

  if (!_ctx.options.count("skip-forgetting"))
  {
    // Initiate walker.
    RevWalkPtr walker = createRevWalk(repo.get());
    git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker.get());

    git_oid oid;
    int allCommits = 0;
    while (git_revwalk_next(&oid, walker.get()) == 0)
      ++allCommits;

    LOG(info) << "[competenceparser] Sampling " << allCommits << " commits in git repository.";

    // TODO: nice function to determine sample size
    int sampleSize = 1;//(int)std::sqrt((double)allCommits);
    LOG(info) << "[competenceparser] Sample size is " << sampleSize << ".";

    RevWalkPtr walker2 = createRevWalk(repo.get());
    git_revwalk_sorting(walker2.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker2.get());

    int commitCounter = 0;
    while (git_revwalk_next(&oid, walker2.get()) == 0)
    {
      ++commitCounter;
      if (commitCounter % sampleSize == 0)
      {
        CommitPtr commit = createCommit(repo.get(), oid);
        CommitJob job(repoPath_, root_, oid, commit.get(), commitCounter);
        sampleCommits(job);
      }
    }
    persistSampleData();
  }
}

void CompetenceParser::traverseCommits(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = createRepository(repoPath_);

  if (!_ctx.options.count("skip-competence"))
  {
    // Initiate walker.
    RevWalkPtr walker = createRevWalk(repo.get());
    git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker.get());

    std::vector<std::pair<git_oid, CommitPtr>> commits;
    git_oid oid;
    int commitCounter = 0;
    while (git_revwalk_next(&oid, walker.get()) == 0 && _maxCommitCount > commitCounter)
    {
      // Retrieve commit.
      CommitPtr commit = createCommit(repo.get(), oid);
      CommitJob job(repoPath_, root_, oid, commit.get(), ++commitCounter);
      _pool->enqueue(job);
    }
    _commitCount = _maxCommitCount;
    _pool->wait();
  }
}

void CompetenceParser::commitWorker(CommitJob& job)
{
  RepositoryPtr repo = createRepository(job._repoPath);

  LOG(info) << "[competenceparser] Parsing " << job._commitCounter << "/" << _commitCount << " of version control history.";

  const git_signature* commitAuthor = git_commit_author(job._commit);
  bool valid = true;
  for (int i = 0; i < 5; ++i)
  {
    valid = std::isgraph(commitAuthor->name[i]);
    if (!valid)
      break;
  }
  if (commitAuthor)
    if (valid)
      LOG(info) << "commit author name:" << commitAuthor->name;
    else
      LOG(info) << "commit author is garbage";
  else
    LOG(info) << "commit author is null";
  const git_signature* committer = git_commit_committer(job._commit);
  if (committer)
    LOG(info) << "commit committer name:" << commitAuthor->name;
  else
    LOG(info) << "commit committer is null";
  // Calculate elapsed time in full months since current commit.
  std::time_t elapsed = std::chrono::system_clock::to_time_t(
    std::chrono::system_clock::now()) - commitAuthor->when.time;
  double months = elapsed / (double) (secondsInDay * daysInMonth);

  //if (_maxCommitHistoryLength > 0 && months > _maxCommitHistoryLength)
    //return;

  // Retrieve parent of commit.
  CommitPtr parent = createParentCommit(job._commit);

  if (!parent)
    return;

  // Get git tree of both commits.
  TreePtr commitTree = createTree(job._commit);
  TreePtr parentTree = createTree(parent.get());

  if (!commitTree || !parentTree)
    return;

  // Calculate diff of trees.
  DiffPtr diff = createDiffTree(repo.get(), parentTree.get(), commitTree.get());

  // Loop through each delta.
  size_t num_deltas = git_diff_num_deltas(diff.get());
  if (num_deltas == 0)
    return;

  // Analyse every file that was affected by the commit.
  for (size_t j = 0; j < num_deltas; ++j)
  {
    const git_diff_delta* delta = git_diff_get_delta(diff.get(), j);
    git_diff_file diffFile = delta->new_file;
    model::FilePtr file = _ctx.srcMgr.getFile(job._root + "/" + diffFile.path);
    if (!file)
      continue;

    float totalLines = 0;

    BlameOptsPtr opt = createBlameOpts(job._oid);
    BlamePtr blame = createBlame(repo.get(), diffFile.path, opt.get());

    if (!blame)
      continue;

    // Store the current number of blame lines for each user.
    std::map<UserEmail, UserBlameLines> userBlame;
    std::uint32_t blameHunkCount = git_blame_get_hunk_count(blame.get());
    for (std::uint32_t i = 0; i < blameHunkCount; ++i)
    {
      const git_blame_hunk* hunk = git_blame_get_hunk_byindex(blame.get(), i);

      if (!git_oid_equal(&hunk->final_commit_id, &job._oid))
      {
        totalLines += hunk->lines_in_hunk;
        continue;
      }

      GitBlameHunk blameHunk;
      blameHunk.linesInHunk = hunk->lines_in_hunk;
      if (hunk->final_signature)
      {
        blameHunk.finalSignature.email = hunk->final_signature->email;
      }
      //else
        //continue;
        // TODO
        // git_oid_iszero is deprecated.
        // It should be replaced with git_oid_is_zero in case of upgrading libgit2.
      else if (!git_oid_iszero(&hunk->final_commit_id))
      {
        CommitPtr newCommit = createCommit(repo.get(), hunk->final_commit_id);
        const git_signature* author = git_commit_author(newCommit.get());
        blameHunk.finalSignature.email = author->email;
      }

      auto it = userBlame.find(std::string(blameHunk.finalSignature.email));
      if (it != userBlame.end())
      {
        it->second += blameHunk.linesInHunk;
      }
      else
      {
        userBlame.insert(std::make_pair(std::string(blameHunk.finalSignature.email),
                                        blameHunk.linesInHunk));
        _calculateFileData.lock();
        _emailAddresses.insert(blameHunk.finalSignature.email);
        _calculateFileData.unlock();
      }

      totalLines += blameHunk.linesInHunk;
    }
    persistCommitData(file->id, userBlame, totalLines, commitAuthor->when.time);
    _calculateFileData.lock();

    for (const auto& pair : userBlame)
    {
      if (pair.second != 0)
      {
        // Calculate the retained memory depending on the elapsed time.
        double percentage = pair.second / totalLines * std::exp(-months) * 100;

        auto fileIter = _fileEditions.end();
        for (auto it = _fileEditions.begin(); it != _fileEditions.end(); ++it)
          if (it->_file.get()->id == file.get()->id)
          {
            fileIter = it;
            break;
          }

        if (fileIter != _fileEditions.end())
        {
          auto userIter = fileIter->_editions.find(pair.first);
          if (userIter != fileIter->_editions.end())
          {
            userIter->second.first += (int)percentage;
            ++(userIter->second.second);
          }
          else
          {
            fileIter->_editions.insert(std::make_pair(pair.first, std::make_pair(percentage, 1)));
          }
        }
        else
        {
          FileEdition fe { file, std::map<UserEmail, FileDataPair>() };
          fe._editions.insert(std::make_pair(pair.first, std::make_pair(percentage, 1)));
          _fileEditions.push_back(fe);
       }
      }
    }
    _calculateFileData.unlock();
  }

  LOG(info) << "[competenceparser] Finished parsing " << job._commitCounter << "/" << _commitCount;
}

void CompetenceParser::persistCommitData(
  const model::FileId& fileId_,
  const std::map<UserEmail, UserBlameLines>& userBlame_,
  const float totalLines_,
  const std::time_t& commitDate_)
{
  util::OdbTransaction transaction(_ctx.db);
  transaction([&, this]
  {
    for (const auto& blame : userBlame_)
    {
      model::CommitData commitData;
      commitData.file = fileId_;
      commitData.committerEmail = blame.first;
      commitData.committedLines = blame.second;
      commitData.totalLines = totalLines_;
      commitData.commitDate = commitDate_;
      _ctx.db->persist(commitData);
    }
  });
}

void CompetenceParser::persistFileComprehensionData()
{
  for (const auto& edition : _fileEditions)
  {
    LOG(info) << "[competenceparser] " << edition._file->path << ": ";
    for (const auto& pair : edition._editions)
    {
      util::OdbTransaction transaction(_ctx.db);
      transaction([&, this]
      {
        model::FileComprehension fileComprehension;
        fileComprehension.file = edition._file->id;
        fileComprehension.userEmail = pair.first;
        fileComprehension.repoRatio = pair.second.first / pair.second.second;
        fileComprehension.userRatio = fileComprehension.repoRatio.get();
        fileComprehension.inputType = model::FileComprehension::InputType::REPO;
        _ctx.db->persist(fileComprehension);

        LOG(info) << pair.first << " " << fileComprehension.repoRatio.get() << "%";
      });
    }
  }
}

util::DirIterCallback CompetenceParser::persistNoDataFiles()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path
    = boost::filesystem::canonical(currPath_);

    if (boost::filesystem::is_regular_file(path) &&
        !fileEditionContains(currPath_))
      _ctx.srcMgr.getFile(currPath_);

    return true;
  };
}

void CompetenceParser::sampleCommits(CommitJob& job_)
{
  RepositoryPtr repo = createRepository(job_._repoPath);

  // Retrieve parent of commit.
  CommitPtr parent = createParentCommit(job_._commit);

  if (!parent)
    return;

  // Get git tree of both commits.
  TreePtr commitTree = createTree(job_._commit);
  TreePtr parentTree = createTree(parent.get());

  if (!commitTree || !parentTree)
    return;

  // Calculate diff of trees.
  DiffPtr diff = createDiffTree(repo.get(), parentTree.get(), commitTree.get());

  // Loop through each delta.
  size_t num_deltas = git_diff_num_deltas(diff.get());
  if (num_deltas == 0)
    return;

  // Analyse every file that was affected by the commit.
  for (size_t j = 0; j < num_deltas; ++j)
  {
    const git_diff_delta *delta = git_diff_get_delta(diff.get(), j);
    git_diff_file diffFile = delta->new_file;
    model::FilePtr file = _ctx.srcMgr.getFile(job_._root + "/" + diffFile.path);
    if (!file)
      continue;

    auto iter = _commitSample.find(file);
    if (iter == _commitSample.end())
      _commitSample.insert({file, 1});
    else
      ++(iter->second);
  }
}

void CompetenceParser::persistSampleData()
{
  for (const auto& pair : _commitSample)
  {
    util::OdbTransaction transaction(_ctx.db);
    transaction([&, this]
    {
      model::SampleData sample;
      sample.file = pair.first->id;
      sample.occurrences = pair.second;
      _ctx.db->persist(sample);
    });
  }
}

bool CompetenceParser::fileEditionContains(const std::string& path_)
{
  for (const auto& fe : _fileEditions)
    if (fe._file->path == path_)
      return true;

  return false;
}


void CompetenceParser::setUserCompany()
{
  util::OdbTransaction transaction(_ctx.db);
  transaction([&, this]
  {
    auto users = _ctx.db->query<model::UserEmail>();
    for (auto& user : users)
      for (const auto& p : _companyList)
        if (user.email.find(p.first) != std::string::npos)
        {
          user.company = p.second;
          _ctx.db->update(user);
          break;
        }
  });
}

void CompetenceParser::persistEmailAddress()
{
  util::OdbTransaction transaction(_ctx.db);
  transaction([&, this]
  {
    auto query = _ctx.db->query<model::UserEmail>();
    for (const auto& user : query)
      if (_emailAddresses.find(user.email) != _emailAddresses.end())
        _emailAddresses.erase(user.email);
  });

  for (const auto& address : _emailAddresses)
  {
    util::OdbTransaction transaction(_ctx.db);
    transaction([&, this]
    {
      model::UserEmail userEmail;
      userEmail.email = address;
      _ctx.db->persist(userEmail);
    });
  }
}

RevWalkPtr CompetenceParser::createRevWalk(git_repository* repo_)
{
  git_revwalk* walker = nullptr;
  int error = git_revwalk_new(&walker, repo_);

  if (error)
    LOG(error) << "Creating revision walker failed: " << error;

  return RevWalkPtr { walker, &git_revwalk_free };
}

RepositoryPtr CompetenceParser::createRepository(
  const boost::filesystem::path& repoPath_)
{
  git_repository* repository = nullptr;
  int error = git_repository_open(&repository, repoPath_.c_str());

  if (error)
    LOG(error) << "Opening repository " << repoPath_ << " failed: " << error;

  return RepositoryPtr { repository, &git_repository_free };
}

BlamePtr CompetenceParser::createBlame(
  git_repository* repo_,
  const std::string& path_,
  git_blame_options* opts_)
{
  git_blame* blame = nullptr;
  int error = git_blame_file(&blame, repo_, path_.c_str(), opts_);

  if (error)
    LOG(error) << "Getting blame object failed: " << error;

  return BlamePtr { blame, &git_blame_free };
}

CommitPtr CompetenceParser::createCommit(
  git_repository* repo_,
  const git_oid& id_)
{
  git_commit* commit = nullptr;
  int error = git_commit_lookup(&commit, repo_, &id_);

  if (error)
    LOG(error) << "Getting commit failed: " << error;

  return CommitPtr { commit, &git_commit_free };
}

CommitPtr CompetenceParser::createParentCommit(
  git_commit* commit_)
{
  git_commit* parent = nullptr;
  int error = git_commit_parent(&parent, commit_, 0);

  if (error)
    LOG(error) << "Getting commit parent failed: " << error;

  return CommitPtr { parent, &git_commit_free };
}

TreePtr CompetenceParser::createTree(
  git_commit* commit_)
{
  git_tree* tree = nullptr;
  int error = git_commit_tree(&tree, commit_);

  if (error)
    LOG(error) << "Getting commit tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

DiffPtr CompetenceParser::createDiffTree(
  git_repository* repo_,
  git_tree* first_,
  git_tree* second_)
{
  git_diff* diff = nullptr;
  int error = git_diff_tree_to_tree(&diff, repo_, first_, second_, nullptr);

  if (error)
    LOG(error) << "Getting commit diff failed: " << error;

  return DiffPtr { diff, &git_diff_free };
}

BlameOptsPtr CompetenceParser::createBlameOpts(const git_oid& newCommitOid_)
{
  git_blame_options* blameOpts = new git_blame_options;
  git_blame_init_options(blameOpts, GIT_BLAME_OPTIONS_VERSION);
  blameOpts->newest_commit = newCommitOid_;
  return BlameOptsPtr { blameOpts };
}

void CompetenceParser::setCompanyList()
{
  _companyList.insert({"amd.com", "AMD"});
  _companyList.insert({"apple.com", "Apple"});
  _companyList.insert({"arm.com", "ARM"});
  _companyList.insert({"ericsson.com", "Ericsson"});
  _companyList.insert({"fujitsu.com", "Fujitsu"});
  _companyList.insert({"harvard.edu", "Harvard"});
  _companyList.insert({"huawei.com", "Huawei"});
  _companyList.insert({"ibm.com", "IBM"});
  _companyList.insert({"inf.elte.hu", "ELTE FI"});
  _companyList.insert({"intel.com", "Intel"});
  _companyList.insert({"microsoft.com", "Microsoft"});
  _companyList.insert({"nokia.com", "Nokia"});
  _companyList.insert({"oracle.com", "Oracle"});
  _companyList.insert({"sony.com", "Sony"});
  _companyList.insert({"samsung.com", "Samsung"});
}

CompetenceParser::~CompetenceParser()
{
  git_libgit2_shutdown();
}

/* These two methods are used by the plugin manager to allow dynamic loading
   of CodeCompass Parser plugins. Clang (>= version 6.0) gives a warning that
   these C-linkage specified methods return types that are not proper from a
   C code.

   These codes are NOT to be called from any C code. The C linkage is used to
   turn off the name mangling so that the dynamic loader can easily find the
   symbol table needed to set the plugin up.
*/
// When writing a plugin, please do NOT copy this notice to your code.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Competence Plugin");

    description.add_options()
      ("commit-count", po::value<int>(),
        "This is a threshold value. It is the number of commits the competence parser"
        "will process if value is given. If both commit-history and commit-count is given,"
        "the value of commit-count will be the threshold value.")
      ("skip-forgetting",
        "If this flag is given, the competence parser will skip the file competence anaysis.")
      ("skip-competence",
        "If this flag is given, the competence parser will only execute the file"
        "frequency calculation, and skip the file competence anaysis.");

    return description;
  }

  std::shared_ptr<CompetenceParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CompetenceParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
