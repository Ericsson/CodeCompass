#include <unordered_map>
#include <memory>
#include <thread>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

#include <cppparser/cppparser.h>

#include "assignmentcollector.h"
#include "clangastvisitor.h"

namespace cc
{
namespace parser
{

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
      : _ctx(ctx_), _context(context_)
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext& context_) override
    {
      static std::unordered_map<model::CppAstNodeId, std::uint64_t>
        mangledNameCache;
      std::unordered_map<const void*, model::CppAstNodeId>
        clangToAstNodeId;

      {
        ClangASTVisitor clangAstVisitor(
          _ctx, _context, mangledNameCache, clangToAstNodeId);
        clangAstVisitor.TraverseDecl(context_.getTranslationUnitDecl());
      }

      {
        AssignmentCollector assignmentCollector(
          _ctx, _context, mangledNameCache, clangToAstNodeId);
        assignmentCollector.TraverseDecl(context_.getTranslationUnitDecl());
      }
    }

  private:
    ParserContext& _ctx;
    clang::ASTContext& _context;
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

void CppParser::worker()
{
  while (true)
  {
    //--- Select nect compilation command ---//

    _mutex.lock();

    if (_index == _compileCommands.size())
    {
      _mutex.unlock();
      break;
    }

    const clang::tooling::CompileCommand& command = _compileCommands[_index];
    std::size_t index = ++_index;

    _mutex.unlock();

    //--- Assemble compiler command line ---//

    std::vector<const char*> commandLine;
    commandLine.reserve(command.CommandLine.size());
    commandLine.push_back("--");
    std::transform(
      command.CommandLine.begin() + 1, // Skip compiler name
      command.CommandLine.end(),
      std::back_inserter(commandLine),
      [](const std::string& s){ return s.c_str(); });

    int argc = commandLine.size();

    std::unique_ptr<clang::tooling::FixedCompilationDatabase> compilationDb(
      clang::tooling::FixedCompilationDatabase::loadFromCommandLine(
        argc,
        commandLine.data()));

    //--- Start the tool ---//

    VisitorActionFactory factory(_ctx);

    BOOST_LOG_TRIVIAL(info)
      << '(' << index << '/' << _compileCommands.size() << ')'
      << " Parsing " << command.Filename;

    clang::tooling::ClangTool tool(*compilationDb, command.Filename);

    tool.run(&factory);
  }
}

CppParser::CppParser(ParserContext& ctx_) : AbstractParser(ctx_)
{
}
  
std::vector<std::string> CppParser::getDependentParsers() const
{
  return std::vector<std::string>{};
}

bool CppParser::parse()
{
  bool success = true;

  for (const std::string& input
    : _ctx.options["input"].as<std::vector<std::string>>())
    if (boost::filesystem::is_regular_file(input))
      success
        = success && parseByJson(input, _ctx.options["threads"].as<int>());

  return success;
}

bool CppParser::parseByJson(
  const std::string& jsonFile_,
  std::size_t threadNum_)
{
  std::string errorMsg;

  std::unique_ptr<clang::tooling::JSONCompilationDatabase> compDb
    = clang::tooling::JSONCompilationDatabase::loadFromFile(
        jsonFile_, errorMsg);

  if (!errorMsg.empty())
  {
    BOOST_LOG_TRIVIAL(error) << errorMsg;
    return false;
  }

  _compileCommands = compDb->getAllCompileCommands();
  _index = 0;

  std::vector<std::thread> workers;
  workers.reserve(threadNum_);

  for (std::size_t i = 0; i < threadNum_; ++i)
    workers.emplace_back(&cc::parser::CppParser::worker, this);

  for (std::thread& worker : workers)
    worker.join();

  return true;
}

CppParser::~CppParser()
{
}

extern "C"
{
  std::shared_ptr<CppParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CppParser>(ctx_);
  }
}

} // parser
} // cc
