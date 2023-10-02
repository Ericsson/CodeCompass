#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <git2.h>

#include <util/parserutil.h>
#include <util/hash.h>
#include <util/logutil.h>

#include <gitparser/gitparser.h>

namespace cc
{
namespace parser
{

GitParser::GitParser(ParserContext& ctx_) : AbstractParser(ctx_)
{
  git_libgit2_init();
}

int GitParser::getSubmodulePaths(git_submodule *sm, const char *smName, void *payload)
{
  //--- Get already found repositories from payload. ---//

  std::map<std::string, std::string> *repoPaths =
      static_cast<std::map<std::string, std::string> *>(payload);

  //--- Find parent of this subrepository. ---//

  auto parent = repoPaths->find("parent");
  if (parent == repoPaths->end())
  {
    LOG(warning) << "Couldn't find parent repository of " << smName;
    return 1;
  }

  std::string parentPath = (*parent).second;
  std::string smPath = parentPath + "/" + git_submodule_path(sm);

  std::map<std::string, std::string> subrepoPaths;
  subrepoPaths.emplace("parent", smPath);

  //--- Open subrepository so we can use foreach. ---//

  git_repository *repoFromSubrepo;
  int error = git_repository_open(&repoFromSubrepo, smPath.c_str());

  if (error)
  {
    const git_error *errDetails = giterr_last();

    LOG(warning) << "Can't open git subrepo at: " << smPath
                 << "! Errcode: " << std::to_string(error)
                 << "! Exception: " << errDetails->message;

    return 1;
  }

  git_submodule_foreach(repoFromSubrepo, getSubmodulePaths, &subrepoPaths);

  //--- Replace parent with real subrepository name. ---//

  subrepoPaths.erase("parent");
  subrepoPaths.emplace(smName, smPath);

  for (auto srPath : subrepoPaths)
  {
    repoPaths->emplace(srPath.first, srPath.second);
  }

  return 0;
}

util::DirIterCallback GitParser::getParserCallback()
{
  std::string wsDir = _ctx.options["workspace"].as<std::string>();
  std::string projDir = wsDir + '/' + _ctx.options["name"].as<std::string>();
  std::string versionDataDir = projDir + "/version";

  return [&, versionDataDir](const std::string& path_)
  {
    boost::filesystem::path mainRepoPath(path_);

    //--- Check for .git folder ---//

    if (!boost::filesystem::is_directory(mainRepoPath) ||
        ".git" != mainRepoPath.filename())
      return true;

    mainRepoPath = boost::filesystem::canonical(mainRepoPath.parent_path());

    //--- Open main repository. ---//

    git_repository *mainRepo;
    int error = git_repository_open(&mainRepo, mainRepoPath.c_str());

    if (error)
    {
      const git_error *errDetails = giterr_last();

      LOG(warning) << "Can't open git repo at: " << mainRepoPath
                   << "! Errcode: " << std::to_string(error)
                   << "! Exception: " << errDetails->message;

      return false;
    }

    //--- Collect current repository and subrepositories. ---//

    std::map<std::string, std::string> repoPaths;
    repoPaths.emplace("parent", mainRepoPath.string());

    //--- Iterate submodules recursively. ---//

    git_submodule_foreach(mainRepo, getSubmodulePaths, &repoPaths);

    repoPaths.erase("parent");
    repoPaths.emplace(mainRepoPath.filename().string(),
                      mainRepoPath.string());

    //--- Iterate through the collected repositories. ---//

    for (auto rPath : repoPaths)
    {
      boost::filesystem::path path(rPath.second);

      LOG(info) << "Git parser found a git repo at: " << path;

      std::string repoId = std::to_string(util::fnvHash(path.string()));
      std::string clonedRepoPath = versionDataDir + "/" + repoId;

      LOG(info) << "GitParser cloning into " << clonedRepoPath;

      //--- Remove folder if exists ---//

      boost::filesystem::remove_all(clonedRepoPath);

      //--- Clone the repo into a bare repo ---//

      git_clone_options opts;
      git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
      opts.bare = true;

      git_repository *out;
      int error =
          git_clone(&out, path.string().c_str(), clonedRepoPath.c_str(), &opts);

      if (error)
      {
        const git_error *errDetails = giterr_last();

        LOG(warning) << "Can't copy git repo from: " << path
                     << " to: " << clonedRepoPath
                     << "! Errcode: " << std::to_string(error)
                     << "! Exception: " << errDetails->message;

        return false;
      }

      //--- Write repositories to repositories.txt. ---//

      boost::property_tree::ptree pt;
      std::string repoFile(versionDataDir + "/repositories.txt");

      if (boost::filesystem::is_regular(repoFile))
        boost::property_tree::read_ini(repoFile, pt);

      pt.put(repoId + ".name", path.filename().string());
      pt.put(repoId + ".path", path.string());
      boost::property_tree::write_ini(repoFile, pt);
    }
    return true;
  };
}

bool GitParser::parse()
{
  for (const std::string& path :
    _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Git parse path: " << path;

    auto cb = getParserCallback();

    /*--- Call non-empty iter-callback for all files
       in the current root directory. ---*/
    try
    {
      util::iterateDirectoryRecursive(path, cb);
    }
    catch (const std::exception& ex_)
    {
      LOG(warning) << "Git parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      LOG(warning) << "Git parser failed with unknown exception!";
    }
  }
  return true;
}

GitParser::~GitParser()
{
  git_libgit2_shutdown();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Git Plugin");
    return description;
  }

  std::shared_ptr<GitParser> make(ParserContext& ctx_)
  {
    return std::make_shared<GitParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
