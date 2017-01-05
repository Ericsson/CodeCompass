// $Id$
// Created by Aron Barath, 2013

#include <javaparser/javaparser.h>
#include <util/util.h>
#include <util/filesystem.h>
#include <util/streamlog.h>
#include <util/environment.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <stdio.h>
#include <memory>
#include <sys/param.h>
#include <stdlib.h>

namespace cc
{
namespace parser
{

JavaParser::JavaParser(std::shared_ptr<model::Workspace> w_)
  : _workspace(w_)
{
  locateRTjar();

  // start java-parser process
  _javaParserProcess = std::make_unique<JavaParserProcess>();
}

bool JavaParser::accept(const std::string& path_)
{
  std::string ext = util::getExtension(path_);
  return ext == "java" || ext == "JAVA";
}

std::future<ParseResult> JavaParser::parse(
  ParseProps parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::string& source_,
  const std::string&,
  const std::vector<std::string>& opts_)
{
  std::vector<std::string> fullCommand(opts_);
  fullCommand.insert(fullCommand.begin(), "javac");
  fullCommand.push_back(source_);

  BuildSourceTargets bs;
  return parse(parseProps_, buildAction_, srcMgr_, fullCommand, bs);
}

std::future<ParseResult> JavaParser::parse(
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager&,
  const std::vector<std::string>& opts_,
  const BuildSourceTargets&)
{
  std::promise<ParseResult> res;

  std::string database = parseProps_.options.at("database");
  std::string projroot;

  if (parseProps_.options.count("projroot"))
  {
    projroot = parseProps_.options.at("projroot");
  }

#ifdef DATABASE_PGSQL
  static const std::string prefix1("pgsql:");
  static const std::string prefix2("postgresql:");

  if(database.compare(0, prefix1.length(), prefix1)!=0
    && database.compare(0, prefix2.length(), prefix2)!=0)
  {
    database = prefix1 + database;
  }
#endif // DATABASE_PGSQL

  // set parsing arguments
  parser::JavaParserArg pArg;
  pArg.database = database;
  pArg.rtJar = _rtJar;
  pArg.buildId = std::to_string(static_cast<long long>(buildAction_->id));
  pArg.sourcepath = projroot;

  for (std::size_t i = 1; i < opts_.size(); ++i)
    pArg.opts.push_back(opts_[i]);

  _deferredParserArgs.push_back(pArg);

  res.set_value(PARSE_DEFERRED);
  return res.get_future();
}

void JavaParser::postParse(
  ParseProps parseProps_,
  std::size_t & numOfSuccess_,
  std::size_t & numOfFail_,
  search::IndexerServiceIf& indexerSvc_)
{
#ifdef DATABASE_SQLITE
  SLog() << " **** close SQLite database" << std::endl;
  _workspace->close();
  SLog() << " **** SQLite database closed" << std::endl;
#endif // DATABASE_SQLITE

  numOfSuccess_ = 0;
  numOfFail_    = 0;

  // parsing
  for (auto& pArg : _deferredParserArgs)
  {
    auto parsingResult = _javaParserProcess->parse(pArg);

    switch (parsingResult)
    {
      case parser::JavaParsingResult::Success:
        ++numOfSuccess_;
      break;

      case parser::JavaParsingResult::Fail:
        ++numOfFail_;
      break;
    }
  }

  // stop java parser
  _javaParserProcess->stop();
  _javaParserProcess.reset(nullptr);

#ifdef DATABASE_SQLITE
  SLog() << " **** reset SQLite database" << std::endl;
  std::string database = parseProps_.options.at("database");
  _workspace->reset(database);
  SLog() << " **** SQLite database reopened" << std::endl;
#endif // DATABASE_SQLITE

  // store statistic data
  _workspace->addStatistics("Java", "Successfully parsed actions", numOfSuccess_);
  _workspace->addStatistics("Java", "Partially parsed actions", numOfFail_);
}

void JavaParser::locateRTjar()
{
  const char* env_classpath = getenv("CLASSPATH");
  const char* env_path = getenv("PATH");
  std::stringstream paths;

#define PATH_SEPARATOR ':'

  if (env_classpath)
  {
    paths << env_classpath;
    if (env_path)
    {
      paths << PATH_SEPARATOR;
    }
  }
  if (env_path)
  {
    paths << env_path;
  }

  const std::string rtJar = "rt.jar";

  std::string path;
  while (std::getline(paths, path, PATH_SEPARATOR))
  {
    std::string fullPath;
    {
      char        fullPathBuff[PATH_MAX + 1];
      if (::realpath(path.c_str(), fullPathBuff) == NULL)
      {
        SLog() << "Skip path: " << path;
        continue;
      }

      fullPath = fullPathBuff;
    }

    std::size_t fullPathLen = fullPath.length();
    std::size_t rtJarLen = rtJar.length();

    if (fullPath.back() == '*')
    {
      // TODO
      SLog() << "path not supported: " << fullPathLen;
      continue;
    }

    std::size_t pos = fullPath.find(rtJar);
    if (pos == std::string::npos ||
        pos < (fullPathLen - rtJarLen))
    {
      // This isn't the jar itself.
      // First try: append to the current path
      std::string tryPath = fullPath + "/" + rtJar;
      if (::access(tryPath.c_str(), R_OK) != 0)
      {
        // Second try: try at /../lib
        tryPath = fullPath + "/../lib/" + rtJar;
        if (::access(tryPath.c_str(), R_OK) != 0)
        {
          continue;
        }
      }

      fullPath = tryPath;
    }

    if (::access(fullPath.c_str(), R_OK) == 0)
    {
      _rtJar = fullPath;
      SLog() << "rt.jar found at " << _rtJar;
      return;
    }
  }

  SLog(util::ERROR) << "rt.jar is not found!";
}

std::string JavaParser::getDefaultTargetPath(const std::string& srcPath_)
{
  return util::getPathAndFileWithoutExtension(srcPath_) + ".class";
}

} // parser  
} // cc

