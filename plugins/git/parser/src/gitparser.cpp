#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <git2.h>

#include <util/parserutil.h>
#include <util/hash.h>

#include <gitparser/gitparser.h>

namespace cc
{
namespace parser
{

GitParser::GitParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  //--- Init git library ---//

  git_libgit2_init();
}

std::vector<std::string> GitParser::getDependentParsers()  const
{
  return std::vector<std::string>{};
}

util::DirIterCallback GitParser::getParserCallback()
{
  std::string projectDataDir = _ctx.options["data-dir"].as<std::string>();
  std::string versionDataDir = projectDataDir + "/version";

  return [&, versionDataDir](const std::string& path_)
  {
    boost::filesystem::path path(path_);

    //--- Check for .git folder ---//

    if (boost::filesystem::is_directory(path) && ".git" == path.filename())
    {
      BOOST_LOG_TRIVIAL(info) << "Git parser found a git repo at: " << path;

      //--- Generate unique repository ---//

      uint64_t    repoId = util::fnvHash(path_);
      std::string clonedRepoPath = versionDataDir + "/" + std::to_string(repoId);

      BOOST_LOG_TRIVIAL(info) << "GitParser cloning into " << clonedRepoPath;

      //--- Remove folder if exists ---//

      //TODO check for change or and delete only if needed or
      //TODO merge if already exists
      boost::filesystem::remove_all(clonedRepoPath);

      //--- Clone the repo into a bare repo ---//

      git_clone_options *opts = new git_clone_options(GIT_CLONE_OPTIONS_INIT);
      opts->bare = true;

      git_repository *out;
      int error = git_clone(&out, path_.c_str(), clonedRepoPath.c_str(), opts);

      if(error)
      {
        const git_error* errDetails = giterr_last();

        BOOST_LOG_TRIVIAL(warning)
          << "Can't copy git repo from: " << path << " to: " << clonedRepoPath
          << "! Errcode: " << std::to_string(error)
          << "! Exception: " << errDetails->message;

        return false;
      }
    }
    return true;
  };
}

bool GitParser::parse()
{
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    BOOST_LOG_TRIVIAL(info) << "Git parse path: " << path;

    auto cb = getParserCallback();

    /*--- Call non-empty iter-callback for all files
       in the current root directory. ---*/
    try
    {
      util::iterateDirectoryRecursive(path, cb);
    }
    catch (std::exception& ex_)
    {
      BOOST_LOG_TRIVIAL(error)
        << "Git parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      BOOST_LOG_TRIVIAL(error)
        << "Git parser failed with unknown exception!";
    }
  }
  return true;
}

extern "C"
{
  std::shared_ptr<GitParser> make(ParserContext& ctx_)
  {
    return std::make_shared<GitParser>(ctx_);
  }
}

} // parser
} // cc
