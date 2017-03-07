#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <util/dbutil.h>
#include <util/logutil.h>

#include <service/gitservice.h>

namespace
{

/**
 * Callback to make per line of diff text.
 */
int gitDiffToStringCallback(
  const git_diff_delta*,
  const git_diff_hunk*,
  const git_diff_line* l_,
  void* payload_)
{
  std::string& ret = *static_cast<std::string*>(payload_);

  if (l_->origin == GIT_DIFF_LINE_CONTEXT ||
      l_->origin == GIT_DIFF_LINE_ADDITION ||
      l_->origin == GIT_DIFF_LINE_DELETION)
  {
    ret += (char)l_->origin;
  }

  ret += std::string(l_->content, l_->content_len);

  return 0;
}

/**
 * Callback to print diff without the actual changes.
 */
int gitDiffToStringCompactCallback(
  const git_diff_delta*,
  const git_diff_hunk*,
  const git_diff_line* l,
  void* payload)
{
  std::string& ret = *static_cast<std::string*>(payload);

  if (l->origin != GIT_DIFF_LINE_CONTEXT &&
      l->origin != GIT_DIFF_LINE_ADDITION &&
      l->origin != GIT_DIFF_LINE_DELETION &&
      l->origin != GIT_DIFF_LINE_CONTEXT &&
      l->origin != GIT_DIFF_LINE_CONTEXT_EOFNL &&
      l->origin != GIT_DIFF_LINE_ADD_EOFNL &&
      l->origin != GIT_DIFF_LINE_DEL_EOFNL &&
      l->origin != GIT_DIFF_LINE_BINARY)
  {
    ret += std::string(l->content, l->content_len);
  }

  return 0;
}

}

namespace cc
{
namespace service
{
namespace git
{

GitServiceHandler::GitServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const boost::program_options::variables_map& config_)
    : _db(db_), _transaction(db_), _config(config_), _datadir(datadir_)
{
  git_libgit2_init();
}

std::string GitServiceHandler::getRepoPath(const std::string& repoId_) const
{
  return *_datadir + "/version/" + repoId_;
}

void GitServiceHandler::getRepositoryList(std::vector<GitRepository>& return_)
{
  namespace fs = ::boost::filesystem;

  fs::path versionDataDir(*_datadir + "/version");

  if (!fs::is_directory(versionDataDir))
    return;

  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(versionDataDir);
       dirIter != endIter;
       ++dirIter)
  {
    if (!fs::is_directory(dirIter->status()))
      continue;

    GitRepository gitRepo;
    gitRepo.id = dirIter->path().filename().string();
    gitRepo.path = dirIter->path().string();

    RepositoryPtr repo = createRepository(gitRepo.id);

    gitRepo.isHeadDetached = git_repository_head_detached(repo.get()) == 1;

    ReferencePtr head = createRepositoryHead(repo.get());

    switch (git_reference_type(head.get()))
    {
      case GIT_REF_SYMBOLIC:
        LOG(warning) << "HEAD is symbolic reference, not supported yet.";
        break;

      case GIT_REF_OID:
        gitRepo.head
           = gitRepo.isHeadDetached
           ? gitOidToString(git_reference_target(head.get()))
           : git_reference_name(head.get());
        break;

      default:
        LOG(warning) << "HEAD reference is not OID, nor symbolic.";
        break;
    }

    return_.push_back(std::move(gitRepo));
  }
}

void GitServiceHandler::getCommit(
  GitCommit& return_,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  CommitPtr commit = createCommit(repo.get(), gitOidFromStr(hexOid_));

  if (commit)
    setCommitData(return_, repoId_, commit.get());
}

void GitServiceHandler::getTag(
  GitTag& return_,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_oid tagId = gitOidFromStr(hexOid_);
  TagPtr tag = createTag(repo.get(), tagId);

  if (!tag)
    return;

  return_.repoId = repoId_;
  return_.oid = hexOid_;
  return_.message = git_tag_message(tag.get());
  return_.name = git_tag_name(tag.get());
  return_.tagger = gitSignatureToString(git_tag_tagger(tag.get()));
  return_.targetOid = gitOidToString(git_tag_target_id(tag.get()));
}

void GitServiceHandler::getCommitListFiltered(
  CommitListFilteredResult& return_,
  const std::string& repoId_,
  const std::string& hexOid_,
  const int32_t count_,
  const int32_t offset_,
  const std::string& filter_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  RevWalkPtr revWalk = createRevWalk(repo.get());

  git_revwalk_sorting(revWalk.get(), GIT_SORT_TIME);

  git_oid hexoId = gitOidFromStr(hexOid_);
  git_revwalk_push(revWalk.get(), &hexoId);

  git_oid oid;
  int32_t i = 0;
  int32_t cnt = 0;
  while (cnt < count_ && git_revwalk_next(&oid, revWalk.get()) != GIT_ITEROVER)
  {
    ++i;

    if (i < offset_)
      continue;

    CommitPtr commit = createCommit(repo.get(), oid);

    GitCommit gcommit;
    setCommitData(gcommit, repoId_, commit.get());

    if (boost::icontains(gcommit.message, filter_) ||
        boost::icontains(gcommit.author, filter_) ||
        boost::icontains(gcommit.committer, filter_))
    {
      return_.result.push_back(gcommit);
      ++cnt;
    }
  }

  return_.newOffset = offset_ + cnt;
  return_.hasRemaining = git_revwalk_next(&oid, revWalk.get()) != GIT_ITEROVER;
}

void GitServiceHandler::getReferenceList(
  std::vector<std::string>& return_,
  const std::string& repoId_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_strarray references;
  git_reference_list(&references, repo.get());

  for (std::size_t i = 0; i < references.count; ++i)
    return_.emplace_back(references.strings[i]);

  git_strarray_free(&references);
}

void GitServiceHandler::getBranchList(
  std::vector<std::string>& return_,
  const std::string& repoId_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_branch_iterator *it;
  git_branch_iterator_new(&it, repo.get(), GIT_BRANCH_ALL);

  git_reference *ref;
  git_branch_t branchType;
  while (!git_branch_next(&ref, &branchType, it))
  {
    const char *branch_name;
    git_branch_name(&branch_name, ref);

    if (branchType == GIT_BRANCH_REMOTE)
      return_.push_back("refs/remotes/" + std::string(branch_name));
    else if (git_branch_is_head(ref) == 1)
      return_.push_back("refs/heads/" + std::string(branch_name));
  }

  git_reference_free(ref);
  git_branch_iterator_free(it);
}

void GitServiceHandler::getTagList(
  std::vector<std::string>& return_,
  const std::string& repoId_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_strarray tags;
  git_tag_list(&tags, repo.get());
  for (std::size_t i = 0; i < tags.count; ++i)
    return_.emplace_back("refs/tags/" + std::string(tags.strings[i]));
  git_strarray_free(&tags);
}

void GitServiceHandler::getReferenceTopObject(
  ReferenceTopObjectResult& return_,
  const std::string& repoId_,
  const std::string& branchName_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_oid oid;
  int error = git_reference_name_to_id(&oid, repo.get(), branchName_.c_str());

  if (error)
    LOG(error) << "Lookup a reference by name failed: " << error;

  return_.oid = gitOidToString(&oid);

  ObjectPtr object = createObject(repo.get(), oid);

  if (object)
    return_.type =
      static_cast<decltype(return_.type)>(git_object_type(object.get()));
}

void GitServiceHandler::getCommitDiffAsString(
  std::string& return_,
  const std::string& repoId_,
  const std::string& hexOid_,
  const GitDiffOptions& options_,
  const bool isCompact_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_oid currCommitId = gitOidFromStr(hexOid_);
  CommitPtr currCommit = createCommit(repo.get(), currCommitId);

  if (!currCommit)
    return;

  TreePtr treeNew = createTree(currCommit.get());

  std::string fromCommitId = options_.fromCommit;
  if (fromCommitId.empty())
  {
    std::vector<std::string> parents = getParents(currCommit.get());

    if (parents.empty())
      return;

    fromCommitId = parents.front();
  }

  git_oid fromCommitOid = gitOidFromStr(fromCommitId);
  CommitPtr fromCommit = createCommit(repo.get(), fromCommitOid);

  TreePtr treeOld = createTree(fromCommit.get());

  git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
  opts.context_lines = options_.contextLines;
  opts.pathspec.count = options_.pathspec.size();
  opts.pathspec.strings = new char*[opts.pathspec.count];
  for (std::size_t i = 0; i < opts.pathspec.count; ++i)
  {
    opts.pathspec.strings[i] = new char[options_.pathspec[i].size() + 1];
    strcpy(opts.pathspec.strings[i], options_.pathspec[i].c_str());
  }

  DiffPtr diff = createDiff(repo.get(), treeOld.get(), treeNew.get(), &opts);
  return_ = gitDiffToString(diff.get(), isCompact_);
  git_strarray_free(&opts.pathspec);
}

git_oid GitServiceHandler::gitOidFromStr(const std::string& hexOid_)
{
  git_oid oid;
  int error = git_oid_fromstr(&oid, hexOid_.c_str());

  if (error)
    LOG(error)
      << "Parse hex object id(" << hexOid_
      << ") into a git_oid has been failed: " << error;

  return oid;
}

std::string GitServiceHandler::gitOidToString(const git_oid* oid_)
{
  char oidstr[GIT_OID_HEXSZ + 1];
  git_oid_tostr(oidstr, sizeof(oidstr), oid_);

  if (!strlen(oidstr))
    LOG(warning) << "Format a git_oid into a string has been failed.";

  return std::string(oidstr);
}

std::string GitServiceHandler::gitSignatureToString(const git_signature* sig_)
{
  return std::string(sig_->name) + " (" + sig_->email + ")";
}

void GitServiceHandler::setCommitData(
  GitCommit& return_,
  const std::string& repoId_,
  git_commit* commit_)
{
  return_.oid = gitOidToString(git_commit_id(commit_));
  return_.repoId = repoId_;
  return_.message = git_commit_message(commit_);
  return_.summary = git_commit_summary(commit_);
  return_.time = git_commit_time(commit_);
  return_.timeOffset = git_commit_time_offset(commit_);

  const git_signature* author = git_commit_author(commit_);
  return_.author = gitSignatureToString(author);

  const git_signature* cmtter = git_commit_committer(commit_);
  return_.committer = gitSignatureToString(cmtter);

  const git_oid* treeId = git_commit_tree_id(commit_);
  return_.treeOid = gitOidToString(treeId);

  return_.parentOids = getParents(commit_);
}

std::vector<std::string> GitServiceHandler::getParents(git_commit* commit_)
{
  std::vector<std::string> parents;

  unsigned int parentCount = git_commit_parentcount(commit_);
  for (unsigned int i = 0; i < parentCount; ++i)
  {
    git_commit* parent;
    git_commit_parent(&parent, commit_, i);
    const git_oid* parentId = git_commit_id(parent);
    parents.push_back(gitOidToString(parentId));
    git_commit_free(parent);
  }

  return parents;
}

std::string GitServiceHandler::gitDiffToString(git_diff* diff_, bool isCompact_)
{
  std::string ret;

  git_diff_line_cb cb = isCompact_
    ? &gitDiffToStringCompactCallback
    : &gitDiffToStringCallback;

  git_diff_print(diff_, GIT_DIFF_FORMAT_PATCH, cb, &ret);

  return ret;
}

RepositoryPtr GitServiceHandler::createRepository(const std::string& repoId_)
{
  std::string repoPath = getRepoPath(repoId_);
  git_repository* repository;
  int error = git_repository_open(&repository, repoPath.c_str());

  if (error)
    LOG(error) << "Opening repository " << repoPath << " failed: " << error;

  return RepositoryPtr { repository, &git_repository_free };
}

ReferencePtr GitServiceHandler::createRepositoryHead(git_repository* repo_)
{
  git_reference* ref;
  int error = git_repository_head(&ref, repo_);

  if (error)
    LOG(error) << "Getting repository head failed: " << error;

  return ReferencePtr { ref, git_reference_free };
}

RevWalkPtr GitServiceHandler::createRevWalk(git_repository* repo)
{
  git_revwalk* walker;
  int error = git_revwalk_new(&walker, repo);

  if (error)
    LOG(error) << "Creating revision walker failed: " << error;

  return RevWalkPtr { walker, &git_revwalk_free };
}

CommitPtr GitServiceHandler::createCommit(
  git_repository *repo_,
  const git_oid& id_)
{
  git_commit* commit;
  int error = git_commit_lookup(&commit, repo_, &id_);

  if (error)
    LOG(error) << "Getting commit failed: " << error;

  return CommitPtr { commit, &git_commit_free };
}

TreePtr GitServiceHandler::createTree(git_commit* commit_)
{
  git_tree* tree;
  int error = git_commit_tree(&tree, commit_);

  if (error)
    LOG(error) << "Getting commit tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

TreePtr GitServiceHandler::createTree(git_repository* repo_, const git_oid& id_)
{
  git_tree* tree;
  int error = git_tree_lookup(&tree, repo_, &id_);

  if (error)
    LOG(error) << "Getting tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

TagPtr GitServiceHandler::createTag(git_repository* repo, const git_oid& oid_)
{
  git_tag* tag;
  int error = git_tag_lookup(&tag, repo, &oid_);

  if (error)
    LOG(error) << "Getting tag failed: " << error;

  return TagPtr { tag, &git_tag_free };
}

ObjectPtr GitServiceHandler::createObject(
  git_repository* repo_,
  const git_oid& oid_)
{
  git_object* obj;
  int error = git_object_lookup(&obj, repo_, &oid_, GIT_OBJ_ANY);

  if (error)
    LOG(error) << "Reference lookup failed: " << error;

  return ObjectPtr { obj, &git_object_free };
}

DiffPtr GitServiceHandler::createDiff(
  git_repository* repo_,
  git_tree* oldTree_,
  git_tree* newTree_,
  git_diff_options* opts_)
{
  git_diff* diff;
  int error = git_diff_tree_to_tree(&diff, repo_, oldTree_, newTree_, opts_);

  if (error)
    LOG(error) << "Create diff failed: " << error;

  return DiffPtr { diff, &git_diff_free };
}

GitServiceHandler::~GitServiceHandler()
{
  git_libgit2_shutdown();
}

} //namespace version
} //namespace service
} //namespace cc
