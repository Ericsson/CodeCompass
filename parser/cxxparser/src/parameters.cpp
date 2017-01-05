#include "parameters.h"

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/Version.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Options.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/Utils.h>
#include <clang/Config/config.h>
// clang/Config/config.h declares CONFIG_H but llvm/Config/llvm-config.h also
// uses this guard, so we have to undef it first
#undef CONFIG_H
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Host.h>

#include <boost/algorithm/string.hpp>

#include <util/streamlog.h>
#include <util/logutil.h>
#include <util/environment.h>

#include <unistd.h>
#include <string>
#include <set>
#include <algorithm>
#include <regex>
#include <iterator>

#define CC_CLANG_RESDIR_POSTFIX "/lib/clang/" CLANG_VERSION_STRING

using FilterMap = std::vector<std::pair<std::regex, unsigned int>>;
using ReplaceMap = std::map<std::string, std::vector<std::string>>;

/**
 *  This map maps some GCC specific options to its CLANG counterpart.
 */
static const ReplaceMap REPLACE_OPTIONS_MAP{
  { "-mips32"    , { "-target", "mips", "-mips32" }},
  { "-mips64"    , { "-target", "mips64", "-mips64" }},
  { "-mpowerpc"  , { "-target", "powerpc" }},
  { "-mpowerpc64", { "-target", "powerpc64" }}
};

/**
 *  These options are ignored. Basically this is a blacklist.
 *  This map contains (option regex, number of arguments) pairs.
 */
static const FilterMap IGNORED_OPTION_MAP{
  { std::regex{ "^-MQ$" }, 1u },
  { std::regex{ "^-MT$" }, 1u },
  { std::regex{ "^-MF$" }, 1u },
  { std::regex{ "^-fsyntax-only$" }, 0u },
  { std::regex{ "^-save-temps$" }, 0u },
  { std::regex{ "^-install_name$" }, 1u },
  { std::regex{ "^-exported_symbols_list$" }, 1u },
  { std::regex{ "^-current_version$" }, 1u },
  { std::regex{ "^-compatibility_version$" }, 1u },
  { std::regex{ "^-init$" }, 1u },
  { std::regex{ "^-e$" }, 1u },
  { std::regex{ "^-seg1addr$" }, 1u },
  { std::regex{ "^-bundle_loader$" }, 1u },
  { std::regex{ "^-multiply_defined$" }, 1u },
  { std::regex{ "^-sectorder$" }, 3u },
  { std::regex{ "^--param$" }, 1u },
  { std::regex{ "^-u$" }, 1u },
  { std::regex{ "^--serialize-diagnostics$" }, 1u },
  { std::regex{ "^-mmultiple$" }, 0u },
  { std::regex{ "^-mupdate$" }, 0u },
  { std::regex{ "^-m(no-)?string$" }, 0u },
  { std::regex{ "^-m(no-)?sdata.*$" }, 0u },
  { std::regex{ "^-mfix-cortex-m3-ldrd$" }, 0u },
  { std::regex{ "^-mthumb-interwork$" }, 0u },
  { std::regex{ "^-fno-var-tracking-assignments" }, 0u },
  { std::regex{ "^-fconserve-stack" }, 0u },
  { std::regex{ "^-fno-delete-null-pointer-checks" }, 0u },
  { std::regex{ "^-ffixed-r2" }, 0u },
  { std::regex{ "^-fno-var-tracking-assignments" }, 0u },
  { std::regex{ "^-fconserve-stack" }, 0u },
  { std::regex{ "^-m(no-)?spe.*" }, 0u },
  { std::regex{ "^-Werror" }, 0u }
};

/**
 *  These options are allowed. Basically this is a white-list.
 *  This map contains (option regex, number of arguments) pairs.
 */
static const FilterMap COMPILE_OPTION_MAP{
  { std::regex{ "-nostdinc" }, 0u },
  { std::regex{ "-include" }, 1u },
  { std::regex{ "-idirafter" }, 1u },
  { std::regex{ "-imacros" }, 1u },
  { std::regex{ "-iprefix" }, 1u },
  { std::regex{ "-isystem" }, 1u },
  { std::regex{ "-iwithprefix" }, 1u },
  { std::regex{ "-iwithprefixbefore" }, 1u },
  { std::regex{ "-O([1-3]|s)?$" }, 0u },
  { std::regex{ "-std=.*" }, 0u },
  { std::regex{ "^-f.*" }, 0u },
  { std::regex{ "-m.*" }, 0u },
  { std::regex{ "^-Wno-.*" }, 0u },
  { std::regex{ "-Wcast-qual" }, 0u },
  { std::regex{ "-Wuninitialized" }, 0u },
  { std::regex{ "^-m(32|64)$" }, 0u },
  { std::regex{ "-write-strings" }, 0u },
  { std::regex{ "-ftrapv-handler" }, 1u },
  { std::regex{ "-mios-simulator-version-min" }, 0u },
  { std::regex{ "-sysroot" }, 1u },
  { std::regex{ "-stdlib" }, 0u },
  { std::regex{ "-target" }, 1u },
  { std::regex{ "-v" }, 0u },
  { std::regex{ "-g" }, 0u },
  { std::regex{ "-c" }, 0u },
  { std::regex{ "-o" }, 1u },
  { std::regex{ "-pipe" }, 0u },
  { std::regex{ "-mmacosx-version-min" }, 0u },
  { std::regex{ "-miphoneos-version-min" }, 0u },
    // The following options are special:
    //   It is possible to write a space between the option and the parameter
    //   but it is optional.
  { std::regex{ "^-iquote(.+)$" }, 0u },
  { std::regex{ "^-iquote$" }, 1u },
  { std::regex{ "^-[DIU](.+)$" }, 0u },
  { std::regex{ "^-[DIU]$" }, 1u },
  { std::regex{ "^-F(.+)$" }, 0u },
  { std::regex{ "^-F$" }, 1u }
};

namespace
{

using namespace cc::parser;

// FIXME: It's ugly, not dynamic and looks like a hack, but it works!
char const* const clangBinPath = LLVM_BINDIR "/clang";

bool replace(
  CompilerArgments::const_iterator& iter,
  const CompilerArgments::const_iterator endIter,
  CompilerArgments& args,
  const ReplaceMap& rmap)
{
  assert(iter != endIter);

  auto riter = rmap.find(*iter);
  if (riter != rmap.end())
  {
    ++iter;
    args.insert(args.end(), riter->second.begin(), riter->second.end());
    return true;
  }

  return false;
}

bool ignore(
  CompilerArgments::const_iterator& iter,
  const CompilerArgments::const_iterator endIter,
  const FilterMap& fmap)
{
  assert(iter != endIter);

  for (const auto& fp : fmap)
  {
    if (std::regex_match(*iter, fp.first))
    {
      std::advance(iter, fp.second + 1);
      return true;
    }
  }

  return false;
};


bool append(
  CompilerArgments::const_iterator& iter,
  const CompilerArgments::const_iterator endIter,
  CompilerArgments& args,
  const FilterMap& fmap)
{
  assert(iter != endIter);

  for (const auto& fp : fmap)
  {
    std::smatch sm;
    if (std::regex_match(*iter, sm, fp.first))
    {
      auto begin = iter;
      std::advance(iter, fp.second + 1);
      args.insert(args.end(), begin, iter);
      return true;
    }
  }

  return false;
};

CompilerArgments filterCommand(const CompilerArgments& arguments_)
{
  CompilerArgments result;

  // The first argument is the compiler command
  auto argIt = arguments_.begin();
  result.push_back(*argIt);
  ++argIt;

  const auto endIter = arguments_.end();
  while (argIt != endIter)
  {
    bool processed =
      replace(argIt, endIter, result, REPLACE_OPTIONS_MAP) ||
      ignore(argIt, endIter, IGNORED_OPTION_MAP) ||
      append(argIt, endIter, result, COMPILE_OPTION_MAP);
    if (!processed)
    {
      // Unknown option or the source file
      SLog() << "Unknown option: " << *argIt;
      result.push_back(*argIt);
      ++argIt;
    }
  }

  return result;
}

std::string getClangResourceDir()
{
  // Try development environment
  std::string resDir = LLVM_BINDIR "/.." CC_CLANG_RESDIR_POSTFIX;
  if (::access(resDir.c_str(), F_OK) == 0)
  {
    // We have a development environment
    return resDir;
  }

  // Try install path
  resDir  = cc::util::Environment::getInstallPath();
  resDir += CC_CLANG_RESDIR_POSTFIX;
  if (::access(resDir.c_str(), F_OK) == 0)
  {
    // We have a binary build env.
    return resDir;
  }

  SLog(cc::util::CRITICAL) << "Clang resource directory not found!";
  return "";
}

} // anonymous

namespace cc
{
namespace parser
{

std::vector<CompilerArgments> processCommandLine(
  CompilerArgments          options_,
  clang::DiagnosticsEngine& diag_)
{
  std::vector<CompilerArgments> aruments;
  options_ = filterCommand(options_);

  // Create a driver
  clang::driver::Driver driver(
      clangBinPath,
      llvm::sys::getDefaultTargetTriple(),
      diag_);

  driver.setCheckInputsExist(false);
  driver.setTitle("CodeCompassDriver");

  // get arguments from command line options
  std::vector<const char*> arguments;
  arguments.push_back(clangBinPath);
  arguments.push_back("-fsyntax-only");
  arguments.push_back("-fno-spell-checking");

  std::string resDir = getClangResourceDir();
  if (!resDir.empty())
  {
    arguments.emplace_back("-resource-dir");
    arguments.emplace_back(resDir.c_str());
  }

  for (std::size_t i = 1; i < options_.size(); ++i)
  {
    arguments.push_back(options_[i].c_str());
  }

  const std::unique_ptr<clang::driver::Compilation> comp(
      driver.BuildCompilation(llvm::ArrayRef<const char*>(arguments)));

  const clang::driver::JobList& jobs = comp->getJobs();
  for (const clang::driver::Job& job : jobs)
  {
    if (!llvm::isa<clang::driver::Command>(job))
    {
      continue;
    }

    // The job should invoke clang again
    const clang::driver::Command& cmd = llvm::cast<clang::driver::Command>(job);
    if (llvm::StringRef(cmd.getCreator().getName()) != "clang")
    {
      continue;
    }

    // Get final arguments
    CompilerArgments res;
    const auto& finalArgs = cmd.getArguments();
    for (std::string arg : finalArgs)
    {
      // CLang adds some leading whitespaces to paths (I don't know why), so
      // we have to remove those characters.
      boost::algorithm::trim(arg);

      res.emplace_back(arg);
    }

    aruments.push_back(std::move(res));
  }

  return aruments;
}

} // parser
} // cc
