#include <boost/filesystem.hpp>

#include <git2.h>

#include <util/parserutil.h>
#include <util/hash.h>
#include <util/logutil.h>

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
  std::string wsDir = _ctx.options["workspace"].as<std::string>();
  std::string projDir = wsDir + '/' + _ctx.options["name"].as<std::string>();
  std::string versionDataDir = projDir + "/version";

  return [&, versionDataDir](const std::string& path_)
  {
    boost::filesystem::path path(path_);

    //--- Check for .git folder ---//

    if (!boost::filesystem::is_directory(path) || ".git" != path.filename())
      return true;

    LOG(info) << "Git parser found a git repo at: " << path;

    std::string clonedRepoPath = versionDataDir + "/"
      + std::to_string(util::fnvHash(path_));

    LOG(info) << "GitParser cloning into " << clonedRepoPath;

    //--- Remove folder if exists ---//

    boost::filesystem::remove_all(clonedRepoPath);

    //--- Clone the repo into a bare repo ---//

    git_clone_options opts(GIT_CLONE_OPTIONS_INIT);
    opts.bare = true;

    git_repository *out;
    int error = git_clone(&out, path_.c_str(), clonedRepoPath.c_str(), &opts);

    if(error)
    {
      const git_error* errDetails = giterr_last();

      LOG(warning)
        << "Can't copy git repo from: " << path << " to: " << clonedRepoPath
        << "! Errcode: " << std::to_string(error)
        << "! Exception: " << errDetails->message;

      return false;
    }

    return true;
  };
}

bool GitParser::parse()
{
  for(const std::string& path :
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
      LOG(warning)
        << "Git parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      LOG(warning)
        << "Git parser failed with unknown exception!";
    }
  }
  return true;
}

GitParser::~GitParser()
{
  git_libgit2_shutdown();
}

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

} // parser
} // cc
