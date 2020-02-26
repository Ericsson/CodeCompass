#include <competenceparser/competenceparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/parserutil.h>
#include <util/odbtransaction.h>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>

#include <memory>

namespace cc
{
namespace parser
{

CompetenceParser::CompetenceParser(ParserContext& ctx_): AbstractParser(ctx_)
{
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
    if(accept(path))
    {
      LOG(info) << "CompetenceParser parse path: " << path;
    }
  }
  return true;
}

void CompetenceParser::loadRepositoryData(std::string& return_,
  const model::FileId& fileId_,
  const std::string& repoId_,
  const std::string& hexOid_,
  const std::string& path_,
  const std::string& user_)
{
  util::OdbTransaction transaction(_ctx.db);

  transaction([&, this](){
    RepositoryPtr repo = createRepository(repoId_);

    if (!repo)
      return;

    BlameOptsPtr opt = createBlameOpts(gitOidFromStr(hexOid_));
    BlamePtr blame = createBlame(repo.get(), path_.c_str(), opt.get());

    std::uint32_t blameLines = 0;
    std::uint32_t totalLines = 0;

    for (std::uint32_t i = 0; i < git_blame_get_hunk_count(blame.get()); ++i)
    {
      const git_blame_hunk* hunk = git_blame_get_hunk_byindex(blame.get(), i);

      GitBlameHunk blameHunk;
      blameHunk.linesInHunk = hunk->lines_in_hunk;
      blameHunk.boundary = hunk->boundary;
      blameHunk.finalCommitId = gitOidToString(&hunk->final_commit_id);
      blameHunk.finalStartLineNumber = hunk->final_start_line_number;

      if (hunk->final_signature)
      {
        blameHunk.finalSignature.name = hunk->final_signature->name;
        //blameHunk.finalSignature.email = hunk->final_signature->email;
        blameHunk.finalSignature.time = hunk->final_signature->when.time;
      }
        // TODO
        // git_oid_iszero is deprecated.
        // It should be replaced with git_oid_is_zero in case of upgrading libgit2.
      else if (!git_oid_iszero(&hunk->final_commit_id))
      {
        CommitPtr commit = createCommit(repo.get(), hunk->final_commit_id);
        const git_signature* author = git_commit_author(commit.get());
        blameHunk.finalSignature.name = author->name;
        //blameHunk.finalSignature.email = author->email;
        blameHunk.finalSignature.time = author->when.time;
      }

      if (blameHunk.finalSignature.time)
      {
        CommitPtr commit = createCommit(repo.get(), hunk->final_commit_id);
      }

      blameHunk.origCommitId = gitOidToString(&hunk->orig_commit_id);
      blameHunk.origPath = hunk->orig_path;
      blameHunk.origStartLineNumber = hunk->orig_start_line_number;
      if (hunk->orig_signature)
      {
        blameHunk.origSignature.name = hunk->orig_signature->name;
        blameHunk.origSignature.time = hunk->orig_signature->when.time;
      }

      if (!user_.compare(std::string(blameHunk.finalSignature.name)))
      {
        blameLines += blameHunk.linesInHunk;
      }

      totalLines += blameHunk.linesInHunk;

      return_ = user_;
    }

    model::FileComprehension fileComprehension;
    fileComprehension.ratio = blameLines / totalLines * 100;
    fileComprehension.file = std::make_shared<model::File>();
    fileComprehension.file->id = fileId_;
    _ctx.db->persist(fileComprehension);
  });
}

RepositoryPtr CompetenceParser::createRepository(const std::string& repoId_)
{
  std::string repoPath = *_datadir + "/version/" + repoId_;
  git_repository* repository = nullptr;
  int error = git_repository_open(&repository, repoPath.c_str());

  if (error)
    LOG(error) << "Opening repository " << repoPath << " failed: " << error;

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

BlameOptsPtr CompetenceParser::createBlameOpts(const git_oid& newCommitOid_)
{
  git_blame_options* blameOpts = new git_blame_options;
  git_blame_init_options(blameOpts, GIT_BLAME_OPTIONS_VERSION);
  blameOpts->newest_commit = newCommitOid_;
  return BlameOptsPtr { blameOpts };
}

git_oid CompetenceParser::gitOidFromStr(const std::string& hexOid_)
{
  git_oid oid;
  int error = git_oid_fromstr(&oid, hexOid_.c_str());

  if (error)
    LOG(error)
      << "Parse hex object id(" << hexOid_
      << ") into a git_oid has been failed: " << error;

  return oid;
}

std::string CompetenceParser::gitOidToString(const git_oid* oid_)
{
  char oidstr[GIT_OID_HEXSZ + 1];
  git_oid_tostr(oidstr, sizeof(oidstr), oid_);

  if (!strlen(oidstr))
    LOG(warning) << "Format a git_oid into a string has been failed.";

  return std::string(oidstr);
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
        ("competence-arg", po::value<std::string>()->default_value("Competence arg"),
          "This argument will be used by the competence parser.");

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
