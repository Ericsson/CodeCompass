#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <vector>
#include <string>
#include <functional>
#include <algorithm>

#include <parser/commondefs.h>
#include <util/threadpool.h>

namespace cc
{
namespace parser
{
  
class ProjectParser;
class FileParser;

class Parser
{
public:
  static Parser& getParser(unsigned int threadPoolSize_ = 0);

  void registerProjectParser(std::shared_ptr<ProjectParser> projParser_);

  void deregisterProjectParser(std::shared_ptr<ProjectParser> projParser_);

  /**
   * Parser selected to parse the project is depent on the argument proj_. See
   * the concrete project parsers, for details. Basically proj_ can be the path
   * of the project (that case the directory parser is selected). It can be a
   * path of an xml file that describes the building options of a project
   * (XML project parser is selected)
   * \return false when no project parser found for that project
   */
  bool parseProject(const std::string& proj_);

  /**
   * requires a funcion pointer with two integer as arguments. First argument
   * will be the number of files parsed already, and the second one will be the
   * number total files. Before parsing the function will be called with zero as
   * first argument.
   */
  void setProgressCallback(std::function<void(int,int)> progressCallback_);

  util::ThreadPool& getThreadPool();

private:
  static void dummyCallBack(int, int);
  
  Parser(unsigned int threadPoolSize_ = 0);

  std::vector<std::shared_ptr<ProjectParser>> _projParsers;

  std::function<void(int,int)> _progressCallback;

  util::ThreadPool _parserPool;
};

} //parser

} //cc

#endif //PARSER_PARSER_H
