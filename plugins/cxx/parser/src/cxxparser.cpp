#include <memory>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <cxxparser/cxxparser.h>
//#include <util/threadpool.h>

#include "clangastvisitor.h"

namespace cc
{
namespace parser
{

template <typename MyVisitor>
class VisitorActionFactory : public clang::tooling::FrontendActionFactory
{
public:
  VisitorActionFactory(ParserContext& ctx_) : _ctx(ctx_)
  {
  }

  clang::FrontendAction* create() override
  {
    return new MyFrontendAction(_ctx);
  }

private:
  class MyConsumer : public clang::ASTConsumer
  {
  public:
    MyConsumer(ParserContext& ctx_, clang::ASTContext& context_)
      : visitor(ctx_, context_)
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext& context_) override
    {
      visitor.TraverseDecl(context_.getTranslationUnitDecl());
    }

  private:
    MyVisitor visitor;
  };

  class MyFrontendAction : public clang::ASTFrontendAction
  {
  public:
    MyFrontendAction(ParserContext& ctx_) : _ctx(ctx_)
    {
    }

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler_, llvm::StringRef) override
    {
      return std::unique_ptr<clang::ASTConsumer>(
        new MyConsumer(_ctx, compiler_.getASTContext()));
    }

  private:
    ParserContext& _ctx;
  };

  ParserContext& _ctx;
};

CxxParser::CxxParser(ParserContext& ctx_) : AbstractParser(ctx_)
{
}
  
std::vector<std::string> CxxParser::getDependentParsers() const
{
  return std::vector<std::string>{};
}

bool CxxParser::parse()
{
  bool success = true;

  for (const std::string& input
    : _ctx.options["input"].as<std::vector<std::string>>())
    if (boost::filesystem::is_regular_file(input))
      success
        = success && parseByJson(input, _ctx.options["threads"].as<int>());

  return success;
}

bool CxxParser::parseByJson(
  const std::string& jsonFile_,
  std::size_t threadNum_) const
{
  //util::ThreadPool threadPool(threadNum_);

  std::string errorMsg;

  std::unique_ptr<clang::tooling::JSONCompilationDatabase> compDb
    = clang::tooling::JSONCompilationDatabase::loadFromFile(
        jsonFile_, errorMsg);

  if (!errorMsg.empty())
  {
    BOOST_LOG_TRIVIAL(error) << errorMsg;
    return false;
  }

  VisitorActionFactory<ClangASTVisitor> factory(_ctx);

  for (const clang::tooling::CompileCommand& cmd
    : compDb->getAllCompileCommands())
  {
//    threadPool.submit([cmd, &factory](){
      std::vector<const char*> commandLine;
      commandLine.reserve(cmd.CommandLine.size());
      commandLine.push_back("--");
      std::transform(
        cmd.CommandLine.begin() + 1, // Skip compiler name
        cmd.CommandLine.end(),
        std::back_inserter(commandLine),
        [](const std::string& s){ return s.c_str(); });

      int argc = commandLine.size();

      std::unique_ptr<clang::tooling::FixedCompilationDatabase> compilationDb(
        clang::tooling::FixedCompilationDatabase::loadFromCommandLine(
          argc,
          commandLine.data()));

      BOOST_LOG_TRIVIAL(info) << "Parsing " << cmd.Filename;

      clang::tooling::ClangTool tool(*compilationDb, cmd.Filename);
      tool.run(&factory);
//    });
  }

  return true;
}

CxxParser::~CxxParser()
{
}

extern "C"
{
  std::shared_ptr<CxxParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CxxParser>(ctx_);
  }
}

} // parser
} // cc
