#include <competenceparser/competenceparser.h>

#include <chrono>
#include <memory>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
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

  if (_ctx.options.count("commit-history"))
  {
    _maxCommitHistoryLength = _ctx.options["commit-history"].as<int>();

    if (_maxCommitHistoryLength < 1)
    {
      LOG(fatal) << "Commit history length is too small!";
      throw std::logic_error("");
    }

    LOG(info) << "[competenceparser] Commit history of " << _maxCommitHistoryLength << " months will be parsed.";
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

    util::OdbTransaction trans(_ctx.db);
    trans([&, this]()
    {
      std::string repoId = std::to_string(util::fnvHash(repoPath.c_str()));
      RepositoryPtr repo = createRepository(repoPath);

      if (!repo)
        return;

      countFileChanges(path, repoPath);
      traverseCommits(path, repoPath);
    });
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
  };
}

void CompetenceParser::countFileChanges(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = createRepository(repoPath_);

  // Initiate walker.
  RevWalkPtr walker = createRevWalk(repo.get());
  git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
  git_revwalk_push_head(walker.get());

  git_oid oid;
  int commitCounter = 0;
  while (git_revwalk_next(&oid, walker.get()) == 0)
  {
    // Retrieve commit.
    CommitPtr commit = createCommit(repo.get(), oid);

    if (_maxCommitCount > 0 && commitCounter > _maxCommitCount)
      break;

    ++commitCounter;

    // Calculate elapsed time in full months since current commit.
    std::time_t elapsed = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now()) - git_commit_time(commit.get());
    double months = elapsed / (double) (secondsInDay * daysInMonth);

    if (_maxCommitHistoryLength > 0 && months > _maxCommitHistoryLength)
      break;

    // Retrieve parent of commit.
    CommitPtr parent = createParentCommit(commit.get());

    if (!parent)
      break;

    // Get git tree of both commits.
    TreePtr commitTree = createTree(commit.get());
    TreePtr parentTree = createTree(parent.get());

    if (!commitTree || !parentTree)
      continue;

    // Calculate diff of trees.
    DiffPtr diff = createDiffTree(repo.get(), parentTree.get(), commitTree.get());

    // Loop through each delta.
    size_t num_deltas = git_diff_num_deltas(diff.get());
    if (num_deltas == 0)
      continue;

    // Analyse every file that was affected by the commit.
    for (size_t j = 0; j < num_deltas; ++j)
    {
      const git_diff_delta* delta = git_diff_get_delta(diff.get(), j);
      git_diff_file diffFile = delta->new_file;

      model::FilePtr file = _ctx.srcMgr.getFile(root_ + diffFile.path);
      if (!file)
        continue;

      auto iter = _changeCount.find(file);
      if (iter != _changeCount.end())
        ++iter->second;
      else
        _changeCount.insert(std::make_pair(file, 1));
    }
  }

  _commitCount = commitCounter;
}

void CompetenceParser::traverseCommits(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = createRepository(repoPath_);

  // Initiate walker.
  RevWalkPtr walker = createRevWalk(repo.get());
  git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
  git_revwalk_push_head(walker.get());

  git_oid oid;
  int commitCounter = 0;
  while (git_revwalk_next(&oid, walker.get()) == 0)
  {
    // Retrieve commit.
    CommitPtr commit = createCommit(repo.get(), oid);

    if (_maxCommitCount > 0 && commitCounter > _maxCommitCount)
      break;

    ++commitCounter;
    LOG(info) << "[competenceparser] Parsing " << commitCounter << "/" << _commitCount << " of version control history.";

    // Calculate elapsed time in full months since current commit.
    std::time_t elapsed = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now()) - git_commit_time(commit.get());
    double months = elapsed / (double) (secondsInDay * daysInMonth);

    if (_maxCommitHistoryLength > 0 && months > _maxCommitHistoryLength)
      break;

    // Retrieve parent of commit.
    CommitPtr parent = createParentCommit(commit.get());

    if (!parent)
      break;

    // Get git tree of both commits.
    TreePtr commitTree = createTree(commit.get());
    TreePtr parentTree = createTree(parent.get());

    if (!commitTree || !parentTree)
      continue;

    // Calculate diff of trees.
    DiffPtr diff = createDiffTree(repo.get(), parentTree.get(), commitTree.get());

    // Loop through each delta.
    size_t num_deltas = git_diff_num_deltas(diff.get());
    if (num_deltas == 0)
      continue;

    // Analyse every file that was affected by the commit.
    for (size_t j = 0; j < num_deltas; ++j)
    {
      const git_diff_delta* delta = git_diff_get_delta(diff.get(), j);
      git_diff_file diffFile = delta->new_file;

      model::FilePtr file = _ctx.srcMgr.getFile(root_ + diffFile.path);
      if (!file)
        continue;

      float totalLines = 0;

      BlameOptsPtr opt = createBlameOpts(oid);
      BlamePtr blame = createBlame(repo.get(), diffFile.path, opt.get());

      if (!blame)
        continue;

      // Store the current number of blame lines for each user.
      std::map<UserEmail, UserBlameLines> userBlame;

      for (std::uint32_t i = 0; i < git_blame_get_hunk_count(blame.get()); ++i)
      {
        const git_blame_hunk* hunk = git_blame_get_hunk_byindex(blame.get(), i);

        GitBlameHunk blameHunk;
        blameHunk.linesInHunk = hunk->lines_in_hunk;

        if (hunk->final_signature)
        {
          blameHunk.finalSignature.email = hunk->final_signature->email;
        }
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
          persistEmailAddress(blameHunk.finalSignature.email);
        }

        totalLines += blameHunk.linesInHunk;
      }

      auto fileCountIter = _changeCount.find(file);
      if (fileCountIter != _changeCount.end())
        --(fileCountIter->second);

      for (const auto& pair : userBlame)
      {
        if (pair.second != 0)
        {
          // Calculate the retained memory depending on the elapsed time.
          double percentage = pair.second / totalLines * std::exp(-months) * 100;

          //auto it = _userEditions.find(pair.first);
          auto fileIter = _userEditions.find(file);
          if (fileIter != _userEditions.end())
          {
            auto userIter = fileIter->second.find(pair.first);
            if (userIter != fileIter->second.end())
            {
              userIter->second.first += (int)percentage;
              ++(userIter->second.second);
            }
            else
            {
              fileIter->second.insert(std::make_pair(pair.first, std::make_pair(percentage, 1)));
            }
          }
          else
          {
            std::pair<model::FilePtr, std::map<UserEmail, FileDataPair>> p =
              { fileCountIter->first, std::map<UserEmail, FileDataPair>() };
            p.second.insert(std::make_pair(pair.first, std::make_pair(percentage, 1)));
            _userEditions.insert(p);
          }
        }
      }
    }

    for (const auto& pair : _changeCount)
    {
      if (pair.second == 0)
      {
        auto it = _userEditions.find(pair.first);
        persistFileComprehensionData(it->first, it->second);
        _changeCount.erase(pair.first);
      }
    }
  }
}

void CompetenceParser::persistFileComprehensionData(
  model::FilePtr file_,
  const std::map<UserEmail, FileDataPair>& userEditions)
{
  util::OdbTransaction transaction(_ctx.db);
  transaction([&, this]
  {
    LOG(info) << "[competenceparser] " << file_.get()->path << ": ";
    for (const auto& pair : userEditions)
    {
      if (pair.second.first !=  0)
      {
        model::FileComprehension fileComprehension;
        fileComprehension.userEmail = pair.first;
        fileComprehension.repoRatio = pair.second.first / pair.second.second;
        fileComprehension.userRatio = fileComprehension.repoRatio.get();
        fileComprehension.file = file_.get()->id;
        fileComprehension.inputType = model::FileComprehension::InputType::REPO;
        _ctx.db->persist(fileComprehension);

        LOG(info) << pair.first << " " << fileComprehension.repoRatio.get() << "%";
      }
    }
  });
}

void CompetenceParser::persistEmailAddress(const std::string& email)
{
  util::OdbTransaction transaction(_ctx.db);
  transaction([&, this]
  {
    auto emails = _ctx.db->query<model::UserEmail>(
        odb::query<cc::model::UserEmail>::email == email);

    if (emails.empty())
    {
      model::UserEmail userEmail;
      userEmail.email = email;
      _ctx.db->persist(userEmail);
    }
  });
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
      ("commit-history", po::value<int>(),
       "This is a threshold value. It is the number of months for which the competence parser"
       "will parse the commit history. If both commit-history and commit-count is given,"
       "the value of commit-count will be the threshold value.")
      ("commit-count", po::value<int>(),
        "This is a threshold value. It is the number of commits the competence parser"
        "will process if value is given. If both commit-history and commit-count is given,"
        "the value of commit-count will be the threshold value.");

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
