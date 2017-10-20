#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

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
    : _db(db_),
      _transaction(db_),
      _config(config_),
      _datadir(datadir_),
      _projectHandler(db_, datadir_, config_)
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

  std::string repoFile(versionDataDir.string() + "/repositories.txt");
  boost::property_tree::ptree pt;

  if (!fs::is_regular(repoFile))
  {
    LOG(warning) << "Repository file not found in data directory: " << repoFile;
    return;
  }

  boost::property_tree::read_ini(repoFile, pt);

  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(versionDataDir);
       dirIter != endIter;
       ++dirIter)
  {
    if (!fs::is_directory(dirIter->status()))
      continue;

    GitRepository gitRepo;
    gitRepo.id = dirIter->path().filename().string();
    gitRepo.path = pt.get<std::string>(gitRepo.id + ".path");
    gitRepo.name = pt.get<std::string>(gitRepo.id + ".name");

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

void GitServiceHandler::getRepositoryByProjectPath(
  RepositoryByProjectPathResult& return_,
  const std::string& path_)
{
  return_.isInRepository = false;

  std::vector<GitRepository> repositories;
  getRepositoryList(repositories);

  for (const GitRepository& repo : repositories)
  {
    if (!boost::starts_with(path_, repo.path))
      continue;

    ReferenceTopObjectResult top;
    getReferenceTopObject(top, repo.id, repo.head);

    if (top.type != GitObjectType::GIT_OBJ_COMMIT) {
      LOG(warning) << "Head is not a commit";
      break;
    }

    std::size_t suffixStart = path_.find('/', repo.path.size() - 1) + 1;
    std::string pathSuffix = path_.substr(suffixStart);

    std::string blob;
    getBlobOidByPath(blob, repo.id, top.oid, pathSuffix);

    if (!blob.empty())
    {
      return_.isInRepository = true;
      return_.repoId = repo.id;
      return_.repoPath = pathSuffix;
      return_.commitId = top.oid;
      return_.activeReference = (repo.isHeadDetached ? "" : repo.head);
      return;
    }
  }
}

void GitServiceHandler::getBlobOidByPath(
  std::string& return_,
  const std::string& repoId_,
  const std::string& hexOid_,
  const std::string& path_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  CommitPtr commit = createCommit(repo.get(), gitOidFromStr(hexOid_));

  std::vector<std::string> pathElements;
  boost::split(pathElements, path_, boost::is_any_of("/"));

  if (pathElements.size() == 1u && pathElements[0].empty())
  {
    const git_oid* treeId = git_commit_tree_id(commit.get());
    return_ = gitOidToString(treeId);
    return;
  }

  TreePtr tree = createTree(commit.get());
  TreeEntryPtr entry = createTreeEntry(tree.get(), path_);

  if (entry)
  {
    const git_oid* oid = git_tree_entry_id(entry.get());
    return_ = gitOidToString(oid);
  }
}

void GitServiceHandler::getBlobContent(
  std::string& return_,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  git_oid oid = gitOidFromStr(hexOid_);
  BlobPtr blob = createBlob(repo.get(), &oid);

  const char* content =
    static_cast<const char*>(git_blob_rawcontent(blob.get()));
  git_off_t size = git_blob_rawsize(blob.get());
  std::string data(content, size);

  return_ = data;
}

void GitServiceHandler::getBlameInfo(
  std::vector<GitBlameHunk>& return_,
  const std::string& repoId_,
  const std::string& hexOid_,
  const std::string& path_,
  const std::string& localModificationsFileId_)
{
  RepositoryPtr repo = createRepository(repoId_);

  if (!repo)
    return;

  BlameOptsPtr opt = createBlameOpts(gitOidFromStr(hexOid_));
  BlamePtr blame = createBlame(repo.get(), path_.c_str(), opt.get());

  if (!localModificationsFileId_.empty())
  {
    std::string fileContent;
    _projectHandler.getFileContent(fileContent, localModificationsFileId_);
    blame = getBlameData(blame, fileContent);
  }

  for (std::uint32_t i = 0; i < git_blame_get_hunk_count(blame.get()); ++i)
  {
    const git_blame_hunk* hunk = git_blame_get_hunk_byindex(blame.get(), i);

    GitBlameHunk blameHunk;
    blameHunk.linesInHunk = hunk->lines_in_hunk;
    blameHunk.boundary = hunk->boundary;
    blameHunk.finalCommitId = gitOidToString(&hunk->final_commit_id);
    blameHunk.finalStartLineNumber = hunk->final_start_line_number;

    // If files are locally changed, final_signature will be null pointer.
    // I think it will be a `libgit2` bug.
    if (hunk->final_signature)
    {
      blameHunk.finalSignature.name = hunk->final_signature->name;
      blameHunk.finalSignature.email = hunk->final_signature->email;
      blameHunk.finalSignature.time = hunk->final_signature->when.time;
    }
    else if (!git_oid_iszero(&hunk->final_commit_id))
    {
      CommitPtr commit = createCommit(repo.get(), hunk->final_commit_id);
      const git_signature* author = git_commit_author(commit.get());
      blameHunk.finalSignature.name = author->name;
      blameHunk.finalSignature.email = author->email;
      blameHunk.finalSignature.time = author->when.time;
    }

    //--- If the changes are not committed yet ---//

    if (blameHunk.finalSignature.time)
    {
      CommitPtr commit = createCommit(repo.get(), hunk->final_commit_id);
      blameHunk.finalCommitMessage = git_commit_message(commit.get());
    }

    blameHunk.origCommitId = gitOidToString(&hunk->orig_commit_id);
    blameHunk.origPath = hunk->orig_path;
    blameHunk.origStartLineNumber = hunk->orig_start_line_number;
    if (hunk->orig_signature)
    {
      blameHunk.origSignature.name = hunk->orig_signature->name;
      blameHunk.origSignature.email = hunk->orig_signature->email;
      blameHunk.origSignature.time = hunk->orig_signature->when.time;
    }

    return_.push_back(std::move(blameHunk));
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
        boost::icontains(gcommit.author.name, filter_) ||
        boost::icontains(gcommit.committer.name, filter_))
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

    if (!parents.empty())
      fromCommitId = parents.front();
  }

  TreePtr treeOld {nullptr, &git_tree_free};
  if (!fromCommitId.empty())
  {
    git_oid fromCommitOid = gitOidFromStr(fromCommitId);
    CommitPtr fromCommit = createCommit(repo.get(), fromCommitOid);
    treeOld = createTree(fromCommit.get());
  }

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

  const git_signature* author = git_commit_author(commit_);
  return_.author.name = author->name;
  return_.author.email = author->email;

  const git_signature* cmtter = git_commit_committer(commit_);
  return_.committer.name = cmtter->name;
  return_.committer.email = cmtter->email;

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
  git_repository* repository = nullptr;
  int error = git_repository_open(&repository, repoPath.c_str());

  if (error)
    LOG(error) << "Opening repository " << repoPath << " failed: " << error;

  return RepositoryPtr { repository, &git_repository_free };
}

ReferencePtr GitServiceHandler::createRepositoryHead(git_repository* repo_)
{
  git_reference* ref = nullptr;
  int error = git_repository_head(&ref, repo_);

  if (error)
    LOG(error) << "Getting repository head failed: " << error;

  return ReferencePtr { ref, git_reference_free };
}

RevWalkPtr GitServiceHandler::createRevWalk(git_repository* repo)
{
  git_revwalk* walker = nullptr;
  int error = git_revwalk_new(&walker, repo);

  if (error)
    LOG(error) << "Creating revision walker failed: " << error;

  return RevWalkPtr { walker, &git_revwalk_free };
}

CommitPtr GitServiceHandler::createCommit(
  git_repository *repo_,
  const git_oid& id_)
{
  git_commit* commit = nullptr;
  int error = git_commit_lookup(&commit, repo_, &id_);

  if (error)
    LOG(error) << "Getting commit failed: " << error;

  return CommitPtr { commit, &git_commit_free };
}

TreePtr GitServiceHandler::createTree(git_commit* commit_)
{
  git_tree* tree = nullptr;
  int error = git_commit_tree(&tree, commit_);

  if (error)
    LOG(error) << "Getting commit tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

TreePtr GitServiceHandler::createTree(git_repository* repo_, const git_oid& id_)
{
  git_tree* tree = nullptr;
  int error = git_tree_lookup(&tree, repo_, &id_);

  if (error)
    LOG(error) << "Getting tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

TreeEntryPtr GitServiceHandler::createTreeEntry(
  git_tree* tree_,
  const std::string& path_)
{
  git_tree_entry *entry = nullptr;
  int error = git_tree_entry_bypath(&entry, tree_, path_.c_str());

  if (error)
  {
    LOG(error) << "Getting tree entry failed: " << error;
  }

  return TreeEntryPtr { entry, &git_tree_entry_free };
}
TagPtr GitServiceHandler::createTag(git_repository* repo, const git_oid& oid_)
{
  git_tag* tag = nullptr;
  int error = git_tag_lookup(&tag, repo, &oid_);

  if (error)
    LOG(error) << "Getting tag failed: " << error;

  return TagPtr { tag, &git_tag_free };
}

ObjectPtr GitServiceHandler::createObject(
  git_repository* repo_,
  const git_oid& oid_)
{
  git_object* obj = nullptr;
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
  git_diff* diff = nullptr;
  int error = git_diff_tree_to_tree(&diff, repo_, oldTree_, newTree_, opts_);

  if (error)
    LOG(error) << "Create diff failed: " << error;

  return DiffPtr { diff, &git_diff_free };
}


BlobPtr GitServiceHandler::createBlob(git_repository* repo_, git_oid* oid_)
{
  git_blob* blob = nullptr;
  int error = git_blob_lookup(&blob, repo_, oid_);

  if (error)
    LOG(error) << "Getting blob object failed: " << error;

  return BlobPtr { blob, &git_blob_free };
}

BlamePtr GitServiceHandler::createBlame(
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

BlamePtr GitServiceHandler::getBlameData(
  const BlamePtr& blame_,
  const std::string content_)
{
  git_blame *blame = nullptr;
  int error = git_blame_buffer(
    &blame,
    blame_.get(),
    content_.data(),
    content_.size());

  if (error)
    LOG(error) << "Getting blame data failed: " << error;

  return BlamePtr { blame, &git_blame_free };
}

BlameOptsPtr GitServiceHandler::createBlameOpts(const git_oid& newCommitOid_)
{
  git_blame_options* blameOpts = new git_blame_options;
  *blameOpts = GIT_BLAME_OPTIONS_INIT;
  blameOpts->newest_commit = newCommitOid_;
  return BlameOptsPtr { blameOpts };
}

GitServiceHandler::~GitServiceHandler()
{
  git_libgit2_shutdown();
}

} //namespace version
} //namespace service
} //namespace cc
