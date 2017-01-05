// $Id$
// Created by Aron Barath, 2013

#ifndef PARSER_JAVAPARSER_JAVAPARSER_H
#define PARSER_JAVAPARSER_JAVAPARSER_H

#include <deque>
#include <unordered_set>

#include <fileparser/fileparser.h>
#include <model/workspace.h>

#include <javaparser-api/javaparserprocess.h>

namespace cc
{
namespace parser
{

class SourceManager;

class JavaParser : public IFileParser
{
public:
  JavaParser(std::shared_ptr<model::Workspace> w_);

  virtual bool accept(const std::string& path_) override;

  virtual std::future<ParseResult> parse(
    ParseProps parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::string& path_,
    const std::string& targetpath_,
    const std::vector<std::string>& opts_) override;

  virtual std::future<ParseResult> parse(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets&) override;

  virtual void postParse(
    ParseProps parseProps_,
    std::size_t & numOfSuccess_,
    std::size_t & numOfFail_,
    search::IndexerServiceIf& indexerSvc_) override;

  virtual std::string getDefaultTargetPath(const std::string& srcPath_) override;

private:
  void locateRTjar();

private:
  std::string _rtJar;
  std::shared_ptr<model::Workspace> _workspace;
  std::unique_ptr<JavaParserProcess> _javaParserProcess;

  std::vector<parser::JavaParserArg> _deferredParserArgs;
};

} // parser
} // cc

#endif // PARSER_JAVAPARSER_JAVAPARSER_H

