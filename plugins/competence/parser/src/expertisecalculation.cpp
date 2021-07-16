#include "competenceparser/expertisecalculation.h"

#include <boost/foreach.hpp>
#include <boost/process.hpp>

#include <chrono>
#include <memory>
#include <cctype>
#include <functional>

#include <model/commitdata.h>
#include <model/commitdata-odb.hxx>
#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/useremail.h>
#include <model/useremail-odb.hxx>
#include <model/filecommitloc.h>
#include <model/filecommitloc-odb.hxx>

#include <parser/sourcemanager.h>

#include <util/hash.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{

std::map<std::pair<std::string, int>, int> ExpertiseCalculation::_fileCommitLocData = {};
GitOperations ExpertiseCalculation::_gitOps;

ExpertiseCalculation::ExpertiseCalculation(ParserContext& ctx_)
  : _ctx(ctx_)
{

}

void ExpertiseCalculation::initialize()
{
  int threadNum = _ctx.options["jobs"].as<int>();
  _pool = util::make_thread_pool<ExpertiseCalculation::CommitJob>(
    threadNum, [this](ExpertiseCalculation::CommitJob& job)
    {
      commitWorker(job);
    });
}

void ExpertiseCalculation::traverseCommits(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = _gitOps.createRepository(repoPath_);

  if (!_ctx.options.count("skip-competence"))
  {
    // Initiate walker.
    RevWalkPtr fileLocWalker = _gitOps.createRevWalk(repo.get());
    git_revwalk_sorting(fileLocWalker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(fileLocWalker.get());

    git_oid fileLocOid;
    int commitCounter = 0;
    _commitCount = _maxCommitCount;
    while (git_revwalk_next(&fileLocOid, fileLocWalker.get()) == 0 && _maxCommitCount > commitCounter)
    {
      CommitPtr commit = _gitOps.createCommit(repo.get(), fileLocOid);
      if (_gitOps.getCommitParentCount(commit.get()) > 1)
        continue;
      CommitJob job(repoPath_, root_, fileLocOid, commit.get(), ++commitCounter);
      collectFileLocData(job);
    }

    for (const auto& data : _fileCommitLocData)
    {
      auto iter = _fileLocData.find(data.first.first);
      if (iter != _fileLocData.end())
      {
        iter->second.push_back(data.second);
      }
      else
      {
        _fileLocData.insert(std::make_pair(data.first.first, std::vector<int>{data.second}));
      }
    }

    auto current = _fileLocData.begin(), next = _fileLocData.begin();
    while (next != _fileLocData.end())
    {
      current = next++;
      if (std::count(current->second.begin(), current->second.end(), 0)
        == (long)current->second.size())
      {
        _fileLocData.erase(current);
      }
      else
      {
        std::sort(current->second.begin(), current->second.end());
      }
    }

    // Initiate walker.
    RevWalkPtr walker = _gitOps.createRevWalk(repo.get());
    git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker.get());

    std::vector<std::pair<git_oid, CommitPtr>> commits;
    git_oid oid;
    commitCounter = 0;
    while (git_revwalk_next(&oid, walker.get()) == 0 && _maxCommitCount > commitCounter)
    {
      // Retrieve commit.
      CommitPtr commit = _gitOps.createCommit(repo.get(), oid);
      if (_gitOps.getCommitParentCount(commit.get()) > 1)
        continue;
      CommitJob job(repoPath_, root_, oid, commit.get(), ++commitCounter);
      _pool->enqueue(job);
    }
    _pool->wait();
  }
}

int ExpertiseCalculation::walkDeltaHunkCb(const char* root,
                                      const git_tree_entry* entry,
                                      void* payload)
{
  WalkDeltaHunkData* data = (WalkDeltaHunkData*)payload;
  const git_oid* entryId = git_tree_entry_id(entry);

  std::string deltaStr(data->delta->new_file.path);
  std::string entryName(root);
  entryName.append(git_tree_entry_name(entry));
  if (deltaStr.compare(entryName) == 0
    && git_tree_entry_filemode(entry) == GIT_FILEMODE_BLOB)
  {
    BlobPtr blob = _gitOps.createBlob(data->repo, entryId);
    std::string text((const char *) git_blob_rawcontent(blob.get()));
    int lineNumber = std::count(text.begin(), text.end(), '\n');

    auto iter = _fileCommitLocData.find({data->delta->new_file.path,
                                         data->commitNumber});
    if (iter != _fileCommitLocData.end())
    {
      iter->second += lineNumber;
    }
    else
    {
      _fileCommitLocData.insert(std::make_pair(
        std::make_pair(data->delta->new_file.path,
                       data->commitNumber),
        lineNumber));
    }
  }
}

void ExpertiseCalculation::collectFileLocData(CommitJob& job)
{
  RepositoryPtr repo = _gitOps.createRepository(job._repoPath);

  //LOG(info) << "[ExpertiseCalculation] Calculating file LOC in " << job._commitCounter
  //        << "/" << _commitCount << " of version control history.";

  // Retrieve parent of commit.
  CommitPtr parent = _gitOps.createParentCommit(job._commit);
  if (!parent)
    return;

  // Get git tree of both commits.
  TreePtr commitTree = _gitOps.createTree(job._commit);
  TreePtr parentTree = _gitOps.createTree(parent.get());
  if (!commitTree || !parentTree)
    return;

  // Calculate diff of trees.
  DiffPtr diff = _gitOps.createDiffTree(repo.get(), parentTree.get(), commitTree.get());
  size_t numDeltas = git_diff_num_deltas(diff.get());
  if (numDeltas == 0)
    return;

  for (size_t i = 0; i < numDeltas; ++i)
  {
    const git_diff_delta* delta = git_diff_get_delta(diff.get(), i);
    WalkDeltaHunkData deltaData = { delta, job._commitCounter, repo.get() };
    git_tree_walk(commitTree.get(), GIT_TREEWALK_PRE, &ExpertiseCalculation::walkDeltaHunkCb, &deltaData);
  }
}

int ExpertiseCalculation::walkCb(const char* root,
                             const git_tree_entry* entry,
                             void* payload)
{
  WalkData* data = (WalkData*)payload;
  if (data->found)
  {
    return 1;
  }

  std::string path(data->delta->new_file.path);
  std::string entryName(git_tree_entry_name(entry));
  std::string current(root + entryName);
  if (path.compare(0, current.size(), current) == 0
    && git_tree_entry_filemode(entry) == GIT_FILEMODE_BLOB)
  {
    if (data->isParent)
      data->basePath.append("/old/");
    else
      data->basePath.append("/new/");
    fs::create_directories(data->basePath);

    BlobPtr blob = _gitOps.createBlob(data->repo, git_tree_entry_id(entry));
    std::ofstream currentFile;
    std::replace(current.begin(), current.end(), '/', '_');
    currentFile.open(data->basePath.string() + current);
    currentFile.write((const char*)git_blob_rawcontent(blob.get()), (size_t) git_blob_rawsize(blob.get()));
    currentFile.close();
    data->found = true;
    //delete entryId;
    //delete data;
    return -1;
  }
  else
  {
    //delete data;
    return 0;
  }
}

std::string ExpertiseCalculation::plagiarismCommand(const std::string& extension)
{
  std::string command("java -jar ../lib/java/jplag-2.12.1.jar -t 1 -vq -l ");

  std::vector<std::string> cppExt = { ".cpp", ".CPP", ".cxx", ".CXX", ".c++", ".C++",
                                      ".c", ".C", ".cc", ".CC", ".h", ".H",
                                      ".hpp", ".HPP", ".hh", ".HH" };
  std::vector<std::string> txtExt = { ".TXT", ".txt", ".ASC", ".asc", ".TEX", ".tex" };
  if (std::find(cppExt.begin(), cppExt.end(), extension) != cppExt.end())
    command.append("c/c++ ");
  else if (extension == ".cs" || extension == ".CS")
    command.append("c#-1.2");
  else if (extension == ".java" || extension == ".JAVA")
    command.append("java19 ");
  else if (extension == ".py")
    command.append("python3 ");
  else if (std::find(txtExt.begin(), txtExt.end(), extension) != txtExt.end())
    command.append("text ");
  else
    return "";

  return command;
}

// TODO
// - refactoring git functions
// - decrease memory leaks
// - add documenting comments
void ExpertiseCalculation::commitWorker(CommitJob& job)
{
  RepositoryPtr repo = _gitOps.createRepository(job._repoPath);

  LOG(info) << "[ExpertiseCalculation] Parsing " << job._commitCounter << "/" << _commitCount << " of version control history.";
  LOG(warning) << "step 1";
  const git_signature* commitAuthor = git_commit_author(job._commit);
  LOG(warning) << "step 2";
  if (commitAuthor && !std::isgraph(commitAuthor->name[0]))
  {
    LOG(info) << "[ExpertiseCalculation] " << job._commitCounter << "/" << _commitCount << " commit author is invalid.";
    return;
  }

  basePath = boost::filesystem::path(_ctx.options["workspace"].as<std::string>());
  basePath.append(_ctx.options["name"].as<std::string>() + "/competence/");
  LOG(warning) << "step 3";

  // Calculate elapsed time in full months since current commit.
  //std::time_t elapsed = std::chrono::system_clock::to_time_t(
  //std::chrono::system_clock::now()) - commitAuthor->when.time;
  //double months = elapsed / (double) (secondsInDay * daysInMonth);
  //double months = elapsed / (double)secondsInDay / 7;

  //if (_maxCommitHistoryLength > 0 && months > _maxCommitHistoryLength)
  //return;

  // Retrieve parent of commit.
  CommitPtr parent = _gitOps.createParentCommit(job._commit);
  LOG(warning) << "step 4";

  if (!parent)
    return;

  // Get git tree of both commits.
  TreePtr commitTree = _gitOps.createTree(job._commit);
  TreePtr parentTree = _gitOps.createTree(parent.get());
  LOG(warning) << "step 5";

  if (!commitTree || !parentTree)
    return;

  // Calculate diff of trees.
  DiffPtr diff = _gitOps.createDiffTree(repo.get(), parentTree.get(), commitTree.get());
  LOG(warning) << "step 6";

  // Loop through each delta.
  size_t num_deltas = git_diff_num_deltas(diff.get());
  if (num_deltas == 0)
    return;
  LOG(warning) << "step 7";

  // Copy all modified files to workspace dir.
  // Old and new version goes to separate directories.
  fs::path outPath(basePath);
  outPath.append(std::to_string(job._commitCounter));
  fs::create_directories(outPath);
  LOG(warning) << "step 7";

  std::vector<const git_diff_delta*> deltas;
  bool hasModifiedFiles = false;
  std::map<fs::path, double> plagValues;
  for (size_t j = 0; j < num_deltas; ++j)
  {
    const git_diff_delta* delta = git_diff_get_delta(diff.get(), j);
    LOG(warning) << "step 8";
    // Walk diff tree to find modified file.
    if (delta->status == GIT_DELTA_MODIFIED)
    {
      hasModifiedFiles = true;
      WalkData commitData = {delta, repo.get(), outPath, false };
      git_tree_walk(commitTree.get(), GIT_TREEWALK_PRE, &ExpertiseCalculation::walkCb, &commitData);
      LOG(warning) << "step 9";

      WalkData parentData = {delta, repo.get(), outPath, true };
      git_tree_walk(parentTree.get(), GIT_TREEWALK_PRE, &ExpertiseCalculation::walkCb, &parentData);
      LOG(warning) << "step 10";
    }

    if (delta->status == GIT_DELTA_ADDED)
    {
      std::string replacedStr(delta->new_file.path);
      std::replace(replacedStr.begin(), replacedStr.end(), '/', '_');
      plagValues.insert(std::make_pair(replacedStr, 0));
    }
    LOG(warning) << "step 11";
  }

  fs::path newPath(outPath);
  newPath.append("/new/");
  fs::path oldPath(outPath);
  oldPath.append("/old/");
  LOG(warning) << "step 12";

  // Determine plagiarism command based on file extension.
  if (hasModifiedFiles && fs::exists(newPath) && fs::exists(oldPath))
  {
    for (const auto &f : fs::directory_iterator(newPath))
    {
      std::string command = plagiarismCommand(fs::extension(f.path()));
      LOG(warning) << "step 13";
      if (command.empty())
      {
        LOG(info) << "Plagiarism detector does not support file type: " << f.path().filename();
        continue;
      }
      LOG(warning) << "step 14";
      command.append("-c " + f.path().string() + " ");
      const fs::recursive_directory_iterator end;
      const auto it = std::find_if(fs::recursive_directory_iterator(oldPath), end,
                                   [&f](const fs::directory_entry &entry)
                                   {
                                     return entry.path().filename() == f.path().filename();
                                   });
      if (it == end)
      {
        // why return???????
        LOG(warning) << "amugy ebbe belefutottam";
        return;
      }
      LOG(warning) << "step 15";

      command.append(it->path().string());
      LOG(warning) << "step 15.1";
      fs::path logPath(outPath);
      LOG(warning) << "step 15.2";
      std::future<std::string> log;
      LOG(warning) << "step 15.3";
      LOG(warning) << command;
      boost::process::system(command, boost::process::std_out > log);
      LOG(warning) << "step 16";

      // Retrieve results from JPlag log (stored in memory).
      std::string logStr = log.get();
      auto index = logStr.find_last_of(' ');
      LOG(warning) << "step 17";

      try
      {
        // std::stod may throw in case of JPlag failure.
        double value = std::stod(logStr.substr(++index));
        plagValues.insert(std::make_pair(f.path().filename(), value));
      }
      catch (std::exception &ex)
      {
        LOG(warning) << "Plagiarism detection unsuccessful (" << ex.what() << "): " << logStr;
      }
    }
    LOG(warning) << "step 18";
    fs::remove_all(outPath);
  }
  else
  {
    LOG(warning) << "Commit " << job._commitCounter << " either has no modified files or "
                                                       " has relevant modified files but "
                                                       " they couldn't be copied to the workspace directory.";
  }
  LOG(warning) << "step 19";
  // Analyse every file that was affected by the commit.
  int hundredpercent = 0;
  for (size_t j = 0; j < num_deltas; ++j)
  {
    const git_diff_delta* delta = git_diff_get_delta(diff.get(), j);
    git_diff_file diffFile = delta->new_file;

    model::FilePtr file = _ctx.srcMgr.getFile(job._root + "/" + diffFile.path);
    if (!file)
      continue;

    double currentPlagValue = -1.0;
    bool irrelevantChange = false;
    if (hasModifiedFiles)
    {
      if (delta->status == GIT_DELTA_MODIFIED
        || delta->status == GIT_DELTA_ADDED)
      {
        std::string pathReplace(delta->new_file.path);
        std::replace(pathReplace.begin(), pathReplace.end(), '/', '_');
        auto iter = plagValues.find(pathReplace);
        if (iter != plagValues.end())
          currentPlagValue = iter->second;
        else
        {
          LOG(warning) << "did not find file: " << pathReplace;
        }
      }

      if (currentPlagValue >= plagThreshold)
      {
        LOG(info) << "Commit " << job._commitCounter << "/" << _commitCount
                  << " did not reach the threshold: " << currentPlagValue;
        ++hundredpercent;
        irrelevantChange = true;
      }
    }

    float totalLines = 0;

    // Get blame for file.
    BlameOptsPtr opt = _gitOps.createBlameOpts(job._oid);
    BlamePtr blame = _gitOps.createBlame(repo.get(), diffFile.path, opt.get());

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
        CommitPtr newCommit = _gitOps.createCommit(repo.get(), hunk->final_commit_id);
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
    persistCommitData(file->id, userBlame, totalLines, static_cast<git_status_t>(delta->status), commitAuthor->when.time);
    if (irrelevantChange)
      continue;
    _calculateFileData.lock();

    for (const auto& pair : userBlame)
    {
      if (pair.second != 0)
      {
        // Calculate the retained memory depending on the elapsed time.
        //double percentage = pair.second / totalLines * std::exp(-months) * 100;
        //auto fileLocIter = _fileLocData.find(delta->new_file.path);
        double strength;
        if (currentPlagValue == -1.0)
        {
          strength = (double)pair.second / (double)totalLines;
        }
        else
        {
          strength = 100 - currentPlagValue;
        }
        /*else if (fileLocIter == _fileLocData.end())
        {
          strength = (double)pair.second; // / (double)totalLines;
        }
        else
        {
          const auto medianIter = fileLocIter->second.begin() + fileLocIter->second.size() / 2;
          std::nth_element(fileLocIter->second.begin(), medianIter , fileLocIter->second.end());
          strength = (double)pair.second; // / (double)*medianIter;
          //strength = (double)pair.second; // / totalLines;
          LOG(info) << "median: " << *medianIter << ", strength: " << strength;
        }*/

        //double percentage = std::exp(-(months/strength)) * 100;
        //double percentage = strength * 100;
        double percentage = strength;

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
            if (userIter->second.first < percentage)
            {
              userIter->second.first = percentage;
              ++(userIter->second.second);
            }
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

  _gitOps.freeSignature(commitAuthor);

  if (num_deltas == (size_t)hundredpercent)
  {
    LOG(warning) << job._commitCounter << "/" << _commitCount << ", " << git_oid_tostr_s(git_commit_id(job._commit))
                 << " had all 100 files.";
  }
  LOG(info) << "[ExpertiseCalculation] Finished parsing " << job._commitCounter << "/" << _commitCount;
}

void ExpertiseCalculation::persistCommitData(
  const model::FileId& fileId_,
  const std::map<UserEmail, UserBlameLines>& userBlame_,
  const float totalLines_,
  git_status_t commitType_,
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
                  commitData.commitType = commitType_;
                  commitData.commitDate = commitDate_;
                  _ctx.db->persist(commitData);
                }
              });
}

void ExpertiseCalculation::persistFileComprehensionData()
{
  for (const auto& edition : _fileEditions)
  {
    LOG(info) << "[ExpertiseCalculation] " << edition._file->path << ": ";
    for (const auto& pair : edition._editions)
    {
      util::OdbTransaction transaction(_ctx.db);
      transaction([&, this]
                  {
                    model::FileComprehension fileComprehension;
                    fileComprehension.file = edition._file->id;
                    fileComprehension.userEmail = pair.first;
                    //LOG(info) << pair.second.first;
                    //fileComprehension.repoRatio = std::exp(-pair.second.first) * 100; // / pair.second.second;
                    fileComprehension.repoRatio = pair.second.first;
                    fileComprehension.userRatio = fileComprehension.repoRatio.get();
                    fileComprehension.inputType = model::FileComprehension::InputType::REPO;
                    _ctx.db->persist(fileComprehension);

                    LOG(info) << pair.first << " " << fileComprehension.repoRatio.get() << "%";
                  });
    }
  }
}

util::DirIterCallback ExpertiseCalculation::persistNoDataFiles()
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

bool ExpertiseCalculation::fileEditionContains(const std::string& path_)
{
  for (const auto& fe : _fileEditions)
    if (fe._file->path == path_)
      return true;

  return false;
}


void ExpertiseCalculation::setUserCompany()
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

void ExpertiseCalculation::persistEmailAddress()
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

void ExpertiseCalculation::setCompanyList()
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
}
}