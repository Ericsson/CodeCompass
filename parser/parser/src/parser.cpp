#include <iostream>

#include <llvm/Support/FileSystem.h>

#include <parser/parser.h>
#include <parser/projectparser.h>

namespace cc
{
namespace parser
{

Parser& Parser::getParser(unsigned int threadPoolSize_)
{
  static Parser instance(threadPoolSize_ > 0 ? threadPoolSize_ :
    std::max(std::thread::hardware_concurrency(), 2u));
  return instance;
}

void Parser::registerProjectParser(std::shared_ptr<ProjectParser> projParser_)
{
  _projParsers.push_back(projParser_);
}

void Parser::deregisterProjectParser(std::shared_ptr<ProjectParser> projParser_)
{
  auto it = std::find(_projParsers.begin(), _projParsers.end(), projParser_);
  if (it != _projParsers.end())
  {
    _projParsers.erase(it);
  }
}

bool Parser::parseProject(const std::string& proj_)
{
  std::cout << "%%%%%%%%%: project parser: " << proj_ << std::endl;

  // check whether the project file exists
  llvm::sys::fs::directory_entry de(proj_);
  llvm::sys::fs::file_status status;

  de.status(status);

  if (status.type() == llvm::sys::fs::file_type::file_not_found)
  {
    std::cerr << "File or directory not found: " << proj_ << std::endl;
    return false;
  }

  if (status.type() == llvm::sys::fs::file_type::status_error)
  {
    std::cerr << "Status error on: " << proj_ << std::endl;
    return false;
  }

  // find the corresponding project parser
  for( std::shared_ptr<ProjectParser> pp : _projParsers )
  {
    if(pp->accept(proj_))
    {
      pp->parse(proj_, _progressCallback);
      return true;
    }
  }

  return false;
}

void Parser::setProgressCallback(std::function<void(int,int)> progressCallback_)
{
  _progressCallback = progressCallback_;
}

util::ThreadPool& Parser::getThreadPool()
{
  return _parserPool;
}

void Parser::dummyCallBack(int, int)
{
}

Parser::Parser(unsigned int threadPoolSize_) :
  _progressCallback(dummyCallBack),
  _parserPool(threadPoolSize_)
{
}

} //parser
} //cc
