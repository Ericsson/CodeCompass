/*
 * versionservice.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: cseri
 */

#include "versionservice.h"

#include <limits>
#include <cctype>
#include <memory>
#include <ctime>
#include <chrono>

#include <boost/algorithm/string.hpp>

#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/query.hxx>

#include <model/option.h>
#include <model/option-odb.hxx>
#include <model/version/repository.h>
#include <model/version/repository-odb.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/threadpool.h>

#include <plugin/servicenotavailexception.h>

#include <gitparser/gitrepository.h>
#include <gitparser/gitobject.h>
#include <gitparser/gitblob.h>
#include <gitparser/gittree.h>
#include <gitparser/gitcommit.h>        
#include <gitparser/githexoid.h>
#include <gitparser/gitreference.h>
#include <gitparser/gitdiff.h>
#include <gitparser/gitblame.h>
#include <gitparser/gittag.h>
#include <gitparser/gitrevwalk.h>



namespace cc
{
namespace service
{
namespace version
{
  
using namespace cc::parser;
using namespace cc::util;

VersionServiceHandler::VersionServiceHandler(
  std::shared_ptr<odb::database> db_,
  const boost::program_options::variables_map& config_)
: _db(db_)
{    
  odb::session s;
  odb::transaction t(_db->begin());
  {  
    typedef odb::result<model::Repository> RepositoryResult;
    RepositoryResult r(_db->query<model::Repository>());
    std::vector<model::Repository> repos(r.begin(), r.end());
    if (repos.empty()) {//if there is no repository exit from constructor
      t.commit();
      throw ServiceNotAvailException("Version service is not available");
    }    
  }
  
  if (config_.count("datadir"))
  {
    _projectDataDir = config_["datadir"].as<std::string>();
  }
  else
  {            
    model::Option option;
    {
      typedef odb::result<model::Option> OptionResult;
      typedef odb::query<model::Option> OptionQuery;
      OptionResult r(_db->query<model::Option>(
          OptionQuery::key == "projectDataDir"));
      std::vector<model::Option> vf(r.begin(), r.end());
  
      if (!vf.empty())
        option = *vf.begin();
    }
    
    _projectDataDir = option.value;
  }

  SLog(INFO) << "VersionService startup with projectDataDir: " << _projectDataDir;
  t.commit();
  
}


namespace {
  std::vector<model::Repository> getRepositoryList_internal(
    std::shared_ptr<odb::database> _db
  )
  {
    odb::session s;
    odb::transaction t(_db->begin());

    typedef odb::result<model::Repository> RepositoryResult;
    //typedef odb::query<model::Repository> RepositoryQuery;
    RepositoryResult r(_db->query<model::Repository>());
    std::vector<model::Repository> vr(r.begin(), r.end());
    return std::move(vr);
  }
}


void VersionServiceHandler::getRepositoryList(
  std::vector<Repository> & _return)
{
  std::vector<model::Repository> vr(getRepositoryList_internal(_db));

  for (auto it = vr.begin(); it != vr.end(); ++it)
  {
    model::Repository &repo = *it;
    Repository repoOut;
    
    repoOut.id = std::to_string(repo.id);
    repoOut.name = repo.name;
    repoOut.path = repo.path;
    repoOut.isHeadDetached = repo.isHeadDetached;
    repoOut.head = repo.head;
    repoOut.pathHash = repo.pathHash;
    _return.push_back(std::move(repoOut));
  }
  
  //cache results
  _repoListCache = _return;
}


std::string VersionServiceHandler::internal_getRepoPath(const std::string& repoId_)
{
  if (_repoListCache.empty()) {
    std::vector<Repository> dummy;
    getRepositoryList(dummy);
  }
  
  for (const Repository &x : _repoListCache)
  {
    if (x.pathHash == repoId_) {
      return _projectDataDir + "/version/" + x.pathHash;
    }
  }
  
  throw std::runtime_error(
    "Repository not found with id " + repoId_);
  
}


void VersionServiceHandler::getBlob(
  VersionBlob& _return,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  GitHexOid h(hexOid_);
  GitBlob b = GitBlob::lookUp(repo, h.toOid());
  
  _return.repoId = repoId_;
  _return.oid = hexOid_;
  _return.data = b.getDataAsString();
}

void VersionServiceHandler::getTree(
  std::vector<VersionTreeEntry> & _return,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  GitHexOid h(hexOid_);
  GitTree t = GitTree::lookUp(repo, h.toOid());
  
  for (size_t i = 0; i < t.getEntryCount(); ++i)
  {
    GitTreeEntry te = t.getEntryByIndex(i);
    
    VersionTreeEntry vte;
    vte.repoId = repoId_;
    vte.fileMode = static_cast<decltype(vte.fileMode)>(te.getMode());
    vte.fileName = te.getNameAsString();
    vte.treeOid = h.str();
    vte.pointedOid = GitHexOid(te.getPointedId()).str();
    _return.push_back(vte);
  }
  
  //move directories to the beginning
  std::stable_sort(_return.begin(), _return.end(), [](const VersionTreeEntry& lhs, const VersionTreeEntry& rhs){
    return
      lhs.fileMode == VersionTreeEntryFileMode::GIT_FILEMODE_TREE &&
      rhs.fileMode != VersionTreeEntryFileMode::GIT_FILEMODE_TREE;    
  });
}

namespace {

void getCommit_internal(
  GitRepository& repo,
  VersionCommit& _return,
  const GitHexOid &hexOid)
{
  GitOid oid = hexOid.toOid();
  GitCommit currCommit(GitCommit::lookUp(repo, oid));

  _return.oid = hexOid.str();
  _return.message = currCommit.getMessage();
  _return.summary = currCommit.getSummary();
  _return.author = currCommit.getAuthor().toString();
  _return.committer = currCommit.getCommitter().toString();
  _return.time = currCommit.getTime();
  _return.timeOffset = currCommit.getTimeOffset();
  //parent commits
  for (size_t i = 0; i < currCommit.getParentCount(); ++i)
  {
    GitOid currParentId = currCommit.getParentId(i);
    GitHexOid currParentHexId(currParentId);
  
    _return.parentOids.push_back(currParentHexId.str());
  }
  _return.treeOid = GitHexOid(currCommit.getTreeId()).str();
}

}

void VersionServiceHandler::getCommit(
  VersionCommit& _return,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  getCommit_internal(repo, _return, GitHexOid(hexOid_));
  _return.repoId = repoId_;
}

void VersionServiceHandler::getTag(
  VersionTag& _return,
  const std::string& repoId_,
  const std::string& hexOid_)
{
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  GitHexOid hexOid(hexOid_);
  GitOid oid = hexOid.toOid();
  GitTag currTag(GitTag::lookUp(repo, oid));

  _return.oid = hexOid.str();
  
  GitOid targetOid = currTag.getTargetId();
  _return.targetOid = GitHexOid(targetOid).str();
  
  _return.message = currTag.getMessage();
  _return.summary = currTag.getSummary();
  _return.name = currTag.getName();
  _return.tagger = currTag.getTagger().toString();
  _return.time = currTag.getTagger().getTime().getTime();
  _return.timeOffset = currTag.getTagger().getTime().getOffset();
  _return.repoId = repoId_;
}

// This class was introduced because
//   1. it allows declaring internal_getRevWalk in the header without
//      including headers for GitOid
//   2. it can be expanded by single-cache mutexes in the future
struct CommitListCache
{
  std::vector<GitOid> oidList;
};

const CommitListCache& VersionServiceHandler::internal_getRevWalk(
  const std::string& repoId_,
  GitRepository& repo,
  const std::string& hexOid_)
{
  std::lock_guard<std::mutex> lock(revWalkCacheMutex);
  
  std::string cacheKey = repoId_ + "##" + hexOid_;
  auto it = revWalkCache.find(cacheKey);
  if (it != revWalkCache.end()) {
    SLog(DEBUG) << "RevWalk - Reusing cached";
    return *it->second;
  } else {
    SLog(DEBUG) << "RevWalk - Creating new";

    //TODO optimize locking by inserting a dummy node first and unlocking
    //the global revWalkCacheMutex while walking a single branch
    std::vector<GitOid> revwalkresult;
  
    GitRevWalk revwalk(repo);
    GitHexOid hexOid(hexOid_);
    revwalk.push(hexOid.toOid());

    std::pair<bool, GitOid> curr;
    while ((curr = revwalk.next()).first)
    {
      
      revwalkresult.push_back(curr.second);
    }
    
    auto insertresult = revWalkCache.insert(std::make_pair(
      cacheKey, std::unique_ptr<CommitListCache>(
        new CommitListCache{revwalkresult}
      )
    ));
    it = insertresult.first;
    
    return *it->second;
  }
}

void VersionServiceHandler::getCommitListFiltered(
  CommitListFilteredResult& _return,
  const std::string& repoId_,
  const std::string& hexOid_,
  const int32_t count_,
  const int32_t offset_,
  const std::string& filterText_)
{
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  
  const CommitListCache& revwalkresult = internal_getRevWalk(repoId_, repo, hexOid_);
  
  //count_ = revwalkresult.size(); //TODO put back input count
  
  //int offset = 0;  //TODO receive offset as input
  int at = offset_;
  int returnedElements = 0;
  int numElements = revwalkresult.oidList.size();
  while (returnedElements < count_ && at < numElements)
  {
    VersionCommit vc;
    vc.repoId = repoId_;
    getCommit_internal(repo, vc, revwalkresult.oidList[at]);
    
    if (
      boost::icontains(vc.message, filterText_) ||
      boost::icontains(vc.author, filterText_) ||
      boost::icontains(vc.committer, filterText_)
    ) {
      _return.result.push_back(vc);
      ++returnedElements;
    }
    
    ++at;
  }
  
  _return.newOffset = at;
  _return.hasRemaining = at < numElements;
}

void VersionServiceHandler::getReferenceList(
  std::vector<std::string> & _return,
  const std::string& repoId_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  _return = GitReference::getList(repo);
}

void VersionServiceHandler::getBrancheList(
  std::vector<std::string> & _return,
  const std::string& repoId_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  _return = GitReference::getBranches(repo);
}

void VersionServiceHandler::getTagList(
  std::vector<std::string> & _return,
  const std::string& repoId_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  _return = GitReference::getTags(repo);
}

void VersionServiceHandler::getReferenceTopObject(
  ReferenceTopObjectResult & _return,
  const std::string& repoId_,
  const std::string& branchName_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  GitOid topId = GitReference::nameToId(repo, branchName_.c_str());
  GitHexOid hexOid(topId);
  _return.oid = hexOid.str();
  
  GitObject obj = GitObject::lookUp(repo, topId);
  _return.type = (VersionObjectType::type) obj.getType();
}


void VersionServiceHandler::getActiveReference(
  std::string & _return,
  const std::string& repoId_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  auto ref = repo.head();
  if (ref->getType() == GitReference::GIT_REF_SYMBOLIC) {
    const char *symTarget = ref->getSymbolicTarget();
    if (symTarget) {
      _return = symTarget;
    } else {
      _return = "A";
    }
  } else {
    _return = "B";
  }
  
  _return += "_";
  _return += std::to_string(ref->getType());
  _return += "_";
  _return += std::to_string(repo.isHeadDetached());
  _return += "_";
  _return += GitHexOid(ref->getTarget()).str();
  
}


namespace
{
  GitDiff getDiffFromOidStr(
    GitRepository &repo,
    std::string hexOid_,
    const VersionDiffOptions& options_,
    bool force_binary_)
  {
    GitHexOid hexOid(hexOid_);
    GitOid oid = hexOid.toOid();
    GitCommit currCommit(GitCommit::lookUp(repo, oid));
    GitTree currTree(currCommit.getTree());
    GitTree parentTree;
    
    if(options_.fromCommit.size()) {
      GitHexOid fromHexOid(options_.fromCommit);
      GitCommit fromCommit(GitCommit::lookUp(repo, fromHexOid.toOid()));
      parentTree = fromCommit.getTree();
    } else {
      if (0 != currCommit.getParentCount()) {
        GitCommit parentCommit(
          GitCommit::lookUp(repo, currCommit.getParentId(0))
        );
        parentTree = parentCommit.getTree();
      }
    }
    
    return GitDiff(
      GitDiff::treeToTreeDiff(
        repo,
        parentTree,
        currTree,
        options_.contextLines,
        force_binary_,
        options_.pathspec)
    );
  }
  
  
  void copyFileInfo(
    VersionDiffDeltaFile &output,
    const parser::GitDiff::GitDiffDeltaFile &input)
  {
    output.fileMode =
      static_cast<VersionTreeEntryFileMode::type>(input.fileMode);
    GitHexOid fileOid(input.fileOid);
    output.fileOid = fileOid.str();
    output.filePath = input.filePath;
    output.fileSize = input.fileSize;
  }
}


void VersionServiceHandler::getCommitDiffAsString(
  std::string & _return,
  const std::string& repoId_,
  const std::string& hexOid_,
  const VersionDiffOptions& options_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  
  GitDiff diff = getDiffFromOidStr(repo, hexOid_, options_, false);
  
  _return = diff.str();
}


void VersionServiceHandler::getCommitDiffAsStringCompact(
  std::string & _return,
  const std::string& repoId_,
  const std::string& hexOid_,
  const VersionDiffOptions& options_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  
  GitDiff diff = getDiffFromOidStr(repo, hexOid_, options_, true);
  
  _return = diff.compactStr();
}


void VersionServiceHandler::getCommitDiffDeltas(
  std::vector<VersionDiffDelta>& _return,
  const std::string& repoId_,
  const std::string& hexOid_,
  const VersionDiffOptions& options_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));

  GitDiff diff = getDiffFromOidStr(repo, hexOid_, options_, false);
  
  auto deltaList = diff.getDeltaList();
  for (auto delta: deltaList)
  {
    _return.emplace_back();
    VersionDiffDelta & newVersionDiffDelta = _return.back();
    
    newVersionDiffDelta.type =
      static_cast<VersionDiffDeltaType::type>(delta.type);
    
    newVersionDiffDelta.isBinary = delta.binary;
    newVersionDiffDelta.isNonBinary = delta.nonBinary;

    copyFileInfo(newVersionDiffDelta.oldFile, delta.oldFile);
    copyFileInfo(newVersionDiffDelta.newFile, delta.newFile);
  }

}


void VersionServiceHandler::getBlobOidByPath(
  std::string& _return,
  const std::string& repoId_,
  const std::string& commidOid_,
  const std::string& path_)
{
  _return = "";
  
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  
  //load commit
  GitHexOid commitHexOid(commidOid_);
  GitOid commitOid = commitHexOid.toOid();
  GitCommit commit = GitCommit::lookUp(repo, commitOid);
  
  //split path
  std::vector<std::string> pathElements;
  boost::split(pathElements, path_, boost::is_any_of("/"));

  
  //walk down the tree

  std::unique_ptr<GitTree> currTree(new GitTree(commit.getTree()));
  std::unique_ptr<GitTree> nextTree;

  if (pathElements.empty()) {
    assert(1 || "boost::split returned an empty vector");
  }
  
  //no path is the root tree
  if (pathElements.size() == 1u && pathElements[0].empty()) {
    GitHexOid ret(currTree->getId());
    _return = ret.str();
    return;
  }

  for (auto it = pathElements.begin(); it != pathElements.end() - 1; ++it)
  {
    if (it->empty()) {
        continue;
    }

    auto entryCount = currTree->getEntryCount();
    for (size_t i = 0; i < entryCount; ++i)
    {
      GitTreeEntry e = currTree->getEntryByIndex(i);
      if (
        *it == e.getName() &&
        e.getMode() == GitTreeEntry::GIT_FILEMODE_TREE)
      {
        //found the proper subtree! load it and save it to nextTree
        nextTree.reset(new GitTree(GitTree::lookUp(repo, e.getPointedId())));
      }
    }    

    if (!nextTree) return;
      
    std::swap(currTree, nextTree);
    nextTree.reset(nullptr);
  }
  
  //find the last entry
  auto entryCount = currTree->getEntryCount();
  for (size_t i = 0; i < entryCount; ++i)
  {
    GitTreeEntry e = currTree->getEntryByIndex(i);
    if (pathElements.back() == e.getName())
    {
      //found the proper element! return its oid
      GitHexOid ret(e.getPointedId());
      _return = ret.str();
      return;
    }
  }    
}


namespace {

std::string loadFileContentFromDatabase(
  std::shared_ptr<odb::database> _db,
  const std::string &fileId_
)
{
  //load file object
  odb::transaction t(_db->begin());
  
  model::FileId fileId = std::stoull(fileId_);
  
  model::File file;
  {
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::File> FileQuery;
    FileResult r(_db->query<model::File>(
        FileQuery::id == fileId));
    std::vector<model::File> vf(r.begin(), r.end());

    if (!vf.empty()) {
      file = vf[0];
      
      //load file content
      return file.content.load()->content;
    } else {
      throw std::runtime_error("Cannot load fileid: " + fileId_);
    }
  }
}
  
}

void VersionServiceHandler::getBlameInfo(
  std::vector<VersionBlameHunk> & _return,
  const std::string& repoId_,
  const std::string& commitOid_,
  const std::string& path_,
  const std::string& localModificationsFileId_)
{
  GitRepository repo(GitRepository::open(internal_getRepoPath(repoId_)));
  
  
  
  GitBlameOptions opts;
  GitHexOid commitHexOid(commitOid_);
  opts.setNewestCommit(commitHexOid.toOid());
  GitBlame b(GitBlame::file(repo, path_.c_str(), opts));
  
  if (localModificationsFileId_.size())
  {
    std::string fileContent = loadFileContentFromDatabase(
      _db,
      localModificationsFileId_
    );
    
    b = b.blameBuffer(fileContent);
  }
  
  for (uint32_t i = 0; i < b.getHunkCount(); ++i)
  {
    GitBlameHunk h = b.getHunkByIndex(i);
    
    _return.emplace_back();
    VersionBlameHunk& newElement = _return.back();
    
    newElement.lines_in_hunk = h.lines_in_hunk;
    newElement.final_commit_id = GitHexOid(h.final_commit_id).str();
    newElement.final_start_line_number = h.final_start_line_number;
    newElement.final_signature.name = h.final_signature.getName();
    newElement.final_signature.email = h.final_signature.getEmail();
    newElement.final_signature.time = h.final_signature.getTime().getTime();
    newElement.orig_commit_id = GitHexOid(h.orig_commit_id).str();
    newElement.orig_path = h.orig_path;
    newElement.orig_start_line_number = h.orig_start_line_number;
    newElement.orig_signature.name = h.orig_signature.getName();
    newElement.orig_signature.email = h.orig_signature.getEmail();
    newElement.orig_signature.time = h.orig_signature.getTime().getTime();
    newElement.boundary = h.boundary;
  }
}

void VersionServiceHandler::getRepositoryByProjectPath(RepositoryByProjectPathResult& _return, const std::string& path_)
{
  _return.isInRepository = false;
  SLog(ERROR) << "getRepositoryByProjectPath path: " << path_;
  SLog(ERROR) << "getRepositoryByProjectPath c: " << _return.commitId;
 
  auto vr = getRepositoryList_internal(_db);

  for (auto &model_repo : vr)
  {  
    if (boost::starts_with(path_, model_repo.path)) {

      auto suffixStart = path_.find('/', model_repo.path.size() - 1) + 1;

      std::string pathSuffix = path_.substr(suffixStart);
      
      //get top commit id of repository
      ReferenceTopObjectResult topObject;
      getReferenceTopObject(
        topObject,
        model_repo.pathHash,
        model_repo.head);

      //top object should be a commit
      if (topObject.type != VersionObjectType::GIT_OBJ_COMMIT) {
        throw std::runtime_error("head is not a commit");
      }
      
      //try to find this node
      std::string resultOfNodeSearch;
      getBlobOidByPath(
        resultOfNodeSearch,
        model_repo.pathHash,
        topObject.oid,
        pathSuffix);
      
      if (resultOfNodeSearch.size())
      {
        _return.isInRepository = true;
        _return.repositoryId = model_repo.pathHash;
        _return.repositoryPath = pathSuffix;
        _return.commitId = topObject.oid;
        _return.activeReference = (model_repo.isHeadDetached ? "" : model_repo.head);
        return;
      }
    }
  }
}

namespace {
  
struct GraphCurrentBranchStruct
{
  GitOid oid;
  std::string color;
  int position;
};
  
}

void VersionServiceHandler::getFileRevisionLog(
  VersionLogResult & _return,
  const std::string& repoId_,
  const std::string& branchName_,
  const std::string& path_,
  const std::string& continueAtCommit_,
  int32_t pageSize_,
  const std::vector<std::string>& continueAtDrawState_
)
{
  ReferenceTopObjectResult to; //TODO look through tags
  getReferenceTopObject(to, repoId_, branchName_);
  
  GitRepository repo = GitRepository::open(internal_getRepoPath(repoId_));
  
  const CommitListCache& revwalkresult = internal_getRevWalk(repoId_, repo, to.oid);
  
  auto revwalkresultit = revwalkresult.oidList.cbegin();
  auto revwalkresultitend = revwalkresult.oidList.cend();

  //advance iterator to desired position if requested
  if (!continueAtCommit_.empty()) {
    GitOid continueAtOid(GitHexOid(continueAtCommit_).toOid());
    
    revwalkresultit = std::find(revwalkresultit, revwalkresultitend, continueAtOid);
    if (revwalkresultit == revwalkresultitend) {
      throw std::runtime_error("Couldn't find continueAtCommit_.");
    }
    
    //continueAtCommit_ has been listed to move to the next commit
    ++revwalkresultit;
  }

  //init graph stuctures
  std::vector<GraphCurrentBranchStruct> graphCurrentBranches;
  //read graph state
  for (const auto& commitOidString : continueAtDrawState_) {
    GraphCurrentBranchStruct currBrach;
    currBrach.oid = GitHexOid(commitOidString.substr(0, 40)).toOid();
    assert(commitOidString[40] == ',');
    currBrach.color = commitOidString.substr(41);
    currBrach.position = 0;
    graphCurrentBranches.push_back(currBrach);
  }
  
  
  
  //walk through revisions
  for (; revwalkresultit != revwalkresultitend; ++revwalkresultit) {
    const GitOid &currCommitOid = *revwalkresultit;
    //load commit and check if relevant
    
    GitCommit currCommit = GitCommit::lookUp(repo, currCommitOid);
    GitTree currTree = currCommit.getTree();
    
    bool relevant = false;

    //check if commit is relevant
    int parentCount = currCommit.getParentCount();
    std::vector<GitOid> relevantParents;
    if (
      path_.empty()
    ) {
      //every commit is relevant it there is no path restriction
      relevant = true;
    } else {      
      switch (parentCount) {
        case 0:
          //an initial commit
          {
            /*GitTree currCommitTree = currCommit.getTree();
            ps.matchTree(currCommitTree);*/

            GitDiff diff = GitDiff::treeToTreeDiff(
              repo,
              GitTree() /* a NULL tree */,
              currTree,
              0,
              true,
              std::vector<std::string>{path_});
            
            relevant = diff.getNumDeltas() > 0;
          }
          break;
        default:
          //normal commit, check in the diff
          //merge commit, check in the diff with all parents
          {
            for (int i = 0; !relevant && i < parentCount; ++i)
            {
              GitCommit parentCommit = currCommit.getParent(i);
              GitTree parentTree = parentCommit.getTree();
              
              GitDiff diff = GitDiff::treeToTreeDiff(
                repo,
                parentTree,
                currTree,
                0,
                true,
                std::vector<std::string>{path_});
              
              bool currRelevant = diff.getNumDeltas() > 0;
              if (currRelevant) {
                relevantParents.push_back(parentCommit.getId());
              }
              relevant = relevant || currRelevant;
            }
          }
          break;
      }
    }
    
    //TODO calculate graph info even if the node is not displayed
    for (int i = 0; i < (int)graphCurrentBranches.size(); ++i) {
      graphCurrentBranches[i].position = i;
    }
    int changeposition = -1;
    std::vector<std::pair<GraphCurrentBranchStruct, GitOid>> graphEndingBranches;
    {
      //search the parent in current branches
      for (int i = 0; i < (int)graphCurrentBranches.size(); ++i) {
        if (graphCurrentBranches[i].oid == currCommitOid) {
          changeposition = i;
        }
      }
      
      if (-1 == changeposition) {
        //unknown branch, create a brand new line from nothing
        changeposition = (int)graphCurrentBranches.size();
        std::string color = GitHexOid(currCommitOid).str().substr(0, 6);
        graphCurrentBranches.push_back(GraphCurrentBranchStruct{currCommitOid, color, changeposition});
      }

      //replace commit with its parents to allow proper wiring later
      GraphCurrentBranchStruct erased = *(graphCurrentBranches.begin() + changeposition);
      graphCurrentBranches.erase(graphCurrentBranches.begin() + changeposition);
      
      {
        //temporary vector to insert all at once
        std::vector<GraphCurrentBranchStruct> currCommitParents;
        currCommitParents.reserve(currCommit.getParentCount());
        for (size_t i = 0; i < currCommit.getParentCount(); ++i) {
          GitOid currParentId = currCommit.getParentId(i);
          //check: if the element is already in the vector, do not insert again
          //its a merge!
          
          bool alreadyexisting = false;
          for (int i = 0; i < (int)graphCurrentBranches.size(); ++i) {
            if (graphCurrentBranches[i].oid == currParentId) {
              if (i > changeposition) {
                // merge existing commit into us
                GraphCurrentBranchStruct xerased = *(graphCurrentBranches.begin() + i);
                graphCurrentBranches.erase(graphCurrentBranches.begin() + i);
                graphEndingBranches.emplace_back(xerased, currParentId);
              } else {
                //merge commit into existing
                graphEndingBranches.emplace_back(erased, currParentId);

                alreadyexisting = true;
              }
              break;
            }
          }
          
          if (!alreadyexisting) {
            std::string color = erased.color.size() ? erased.color : GitHexOid(currParentId).str().substr(0, 6);
            erased.color = "";
            currCommitParents.push_back(GraphCurrentBranchStruct{currParentId, color, changeposition});
          }
        }
        
        graphCurrentBranches.insert(
          graphCurrentBranches.begin() + changeposition,
          currCommitParents.begin(),
          currCommitParents.end()
        );
      }
      
      //SLog(ERROR) << "changeposition: " << changeposition;
      //std::ostringstream ss;
      //for (const GraphCurrentBranchStruct& o:graphCurrentBranches) {
      //  ss << GitHexOid(o.oid).str() << ", ";
      //}
      //SLog(ERROR) << ss.str();
      //std::ostringstream ss2;
      //for (const auto& o:graphEndingBranches) {
      //  ss2 << GitHexOid(o.first.oid).str() << "->" << GitHexOid(o.second).str() <<  ", ";
      //}
      //SLog(ERROR) << "Ending: " << ss2.str();
      
    }
    
    if (relevant) {
      VersionLogEntry e;
      
      //experimental graph support works only without filtering
      if (path_.empty()) {
        //drawing data
        e.drawinfo.dot = changeposition + 1;
        e.drawinfo.dotStyle = parentCount > 1 ? 2 : 1;
        for (int i = 0; i < (int)graphCurrentBranches.size(); ++i) {
          VersionLogEntryDrawinfoEntry ee;
          ee.from = 1 + graphCurrentBranches[i].position;
          ee.to = 1 + i;
          ee.color = graphCurrentBranches[i].color;
          e.drawinfo.l.push_back( ee );
        }
        for (const auto& x : graphEndingBranches) {
          GraphCurrentBranchStruct from = x.first;
          GitOid endingIn = x.second;
          
          //search endingin
          for (int i = 0; i < (int)graphCurrentBranches.size(); ++i) {
            if (graphCurrentBranches[i].oid == endingIn) {
              VersionLogEntryDrawinfoEntry ee;
              ee.from = 1 + from.position;
              ee.to = 1 + i;
              ee.color = from.color;
              e.drawinfo.l.push_back( ee );
            }
          }
        }
      }
      
      //commit data
      getCommit_internal(repo, e.commit, currCommit.getId());
      e.commit.repoId = repoId_;
      _return.logList.push_back(e);
    }
    
    if (static_cast<int32_t>(_return.logList.size()) == pageSize_) {
      break;
    }
  }
  
  _return.drawState.reserve(graphCurrentBranches.size());
  for (const auto& currBranch : graphCurrentBranches) {
    _return.drawState.push_back(
      GitHexOid(currBranch.oid).str() +
      "," +
      currBranch.color);
  }
}

//must define destructor here because CommitListCache is defined
//in the implementation only
VersionServiceHandler::~VersionServiceHandler()
{
}

} // version
} // service
} // cc


