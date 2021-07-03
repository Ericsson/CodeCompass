#include "competenceparser/competenceparser.h"

#include <boost/foreach.hpp>

#include <chrono>
#include <memory>
#include <functional>

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

CompetenceParser::CompetenceParser(ParserContext& ctx_): AbstractParser(ctx_),
  _expertise(ctx_), _sampler(ctx_)
{
  srand(time(nullptr));

  _expertise.initialize();

  _expertise.setCompanyList();

  if (_ctx.options.count("commit-count"))
  {
    _expertise._maxCommitCount = _ctx.options["commit-count"].as<int>();

    if (_expertise._maxCommitCount < 1)
    {
      throw std::logic_error("Commit count is too small!");
    }
    LOG(info) << "[competenceparser] Commit history of " << _expertise._maxCommitCount << " commits will be parsed.";
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
    RepositoryPtr repo = _expertise._gitOps.createRepository(repoPath);

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

    _sampler.commitSampling(path, repoPath);
    _expertise.traverseCommits(path, repoPath);
    _expertise.persistFileComprehensionData();

    auto pcb = _expertise.persistNoDataFiles();

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

    _expertise.persistEmailAddress();
    _expertise.setUserCompany();
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

CompetenceParser::~CompetenceParser()
{
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
