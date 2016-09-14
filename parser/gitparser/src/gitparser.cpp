#include "gitparser/gitparser.h"

#include <boost/filesystem.hpp>

#include <util/streamlog.h>
#include <util/util.h>

#include "llvm/Support/FileSystem.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitreference.h"
#include "gitparser/gitclone.h"
#include "gitparser/githexoid.h"
#include "gitparser/gitexception.h"

#include <model/version/repository.h>
#include <model/version/repository-odb.hxx>
#include <model/workspace.h>


using namespace cc::util;
using namespace cc::model;

namespace cc
{
namespace parser
{
  
namespace {
  
std::string calcRepoShortNameFromPath(const std::string& path)
{
  std::string ret;
  if (!path.empty() && path != "/") {
    auto itend = path.end();
    if ('/' == path.back()) --itend;
    
    auto itbegin = itend - 1;
    while (true)
    {
      if (*itbegin == '/') {
        ++itbegin;
        break;
      }
      if (itbegin == path.begin()) {
        break;
      } else {
        --itbegin;
      }
    }
    
    ret.assign(itbegin, itend);
  }
  return ret;
}

  
}


GitParser::GitParser(std::shared_ptr<model::Workspace> w_) :
  _workspace(w_), _counter(0)
{
  SLog(DEBUG) << "GitParser constructed";
}

void GitParser::beforeTraverse(
    const Traversal::OptionMap& opt_,
    SourceManager&)
{
  auto it = opt_.find("projectDataDir");
  if (it == opt_.end())
  {
    SLog(WARNING) << "GitParser exits - no projectDataDir received";
    _projectDataDir.clear();
  }

  _projectDataDir = it->second;
}

Traversal::DirIterCallback GitParser::traverse(
    const std::string& path_,
    SourceManager& srcMgr_)
{
  if (_projectDataDir.empty())
  {
    SLog(WARNING) << "GitParser: skipping " << path_;
    return Traversal::DirIterCallback();
  }
  
  std::string versionDataDir = _projectDataDir + "/version";

  SLog(DEBUG) << "GitParser traverse main function " << path_;
  
  return [this, versionDataDir, &srcMgr_](
    const std::string& currPath, Traversal::FileType currType)
  {
    try
    {
      if (currType == Traversal::FileType::Directory)
      {  
        //check for .git folder inside.
        //(since it is a hidden folder, it is not traversed)
        
        std::string dotGitFolderPath = currPath + "/.git";
        if (
          llvm::sys::fs::exists(dotGitFolderPath) &&
          llvm::sys::fs::is_directory(dotGitFolderPath))
        {
          SLog(INFO) << "GitParser Found a git repo at " << currPath;
          
          //Check database if this already has a name
          std::vector<Repository> queryResultVec;
          {
            //auto tr = this->_workspace->getTransaction();
            typedef odb::query<Repository> RepositoryQuery;
            auto queryResult = this->_workspace->getDb()->query<Repository>(RepositoryQuery::path == currPath);
            queryResultVec = std::vector<Repository>(queryResult.begin(), queryResult.end());
          }

          Repository db_repo;
          //This variable contains if the db_repo object should be saved after
          //a successful repo cloning
          bool db_repo_save = false;
          
          if (!queryResultVec.empty()) {
            if (queryResultVec.size() > 1) {
              throw std::logic_error("repo path unique constraint error");
            }
            
            db_repo = queryResultVec[0];
          } else {
            //not to fancy but easy and fast way to generate unique repository
            //names is to use the hash of the repository path
            uint64_t repoId = fnvHash(currPath);
            std::string repoPathHash = std::to_string(repoId);
            std::string repoName = calcRepoShortNameFromPath(currPath);
            
            db_repo.name = repoName;
            db_repo.path = currPath;
            db_repo.pathHash = repoPathHash;
            
            GitRepository repo = GitRepository::open(currPath.c_str());
            bool headDetached = repo.isHeadDetached();
            auto headRef = repo.head();
            switch (headRef->getType())
            {
              case GitReference::GIT_REF_SYMBOLIC:
                throw
                  std::runtime_error("Head is symbolic reference, not yet supported");
                break;
              case GitReference::GIT_REF_OID:
                db_repo.isHeadDetached = headDetached;
                if (headDetached)
                {
                  GitHexOid hexOid(headRef->getTarget());
                  db_repo.head = hexOid.str();
                }
                else
                {
                  db_repo.head = std::string(headRef->getName());
                }
                break;
              default:
                throw
                  std::runtime_error("Head reference nor oid, nor symbolic");
            }
            
            db_repo_save = true;
          }
          
          std::string clonedRepoPath = versionDataDir + "/" + db_repo.pathHash;
          SLog(INFO) << "GitParser cloning into " << clonedRepoPath;
          
          //Remove folder if exists
          //TODO check for change or and delete only if needed or
          //TODO merge if already exists
          boost::filesystem::remove_all(clonedRepoPath);

          //Clone the repo into a bare repo
          GitClone cloneOperation;
          cloneOperation.setBare(true);
          cloneOperation.clone(currPath.c_str(), clonedRepoPath.c_str());
       
          
          //Save this repo to the database if needed
          //TODO might be better to do this in the endTravarse method.
          if (db_repo_save) {
            //auto tr = this->_workspace->getTransaction();
            this->_workspace->getDb()->persist(db_repo);
          }
        }
      }
    }
    catch (const GitException& ex)
    {
      SLog(util::CRITICAL) << "Something bad happened: " << ex.what();
      return false;
    }
    
    return true;
  };
}
  
void GitParser::endTraverse(const std::string&, SourceManager&)
{
  SLog(DEBUG) << "GitParser exiting";
}

GitParser::~GitParser() {}

} //namespace parser
} //namespace cc
