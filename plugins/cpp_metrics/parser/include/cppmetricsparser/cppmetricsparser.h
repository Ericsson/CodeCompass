#ifndef CC_PARSER_CPPMETRICSPARSER_H
#define CC_PARSER_CPPMETRICSPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>

#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>

#include <util/parserutil.h>
#include <util/threadpool.h>

namespace cc
{
namespace parser
{
  
class CppMetricsParser : public AbstractParser
{
public:
  CppMetricsParser(ParserContext& ctx_);
  virtual ~CppMetricsParser();

  virtual bool cleanupDatabase() override;
  virtual bool parse() override;

private:
  // Calculate the count of parameters for every function.
  void functionParameters();
  // Calculate the McCabe complexity of functions.
  void functionMcCabe();
  // Calculate the bumpy road metric for every function.
  void functionBumpyRoad();
  // Calculate the lack of cohesion between member variables
  // and member functions for every type.
  void lackOfCohesion();
  // Calculate the cohesion within modules
  void relationalCohesion();
  // Check type relations in template parameter view.
  // Used in relational cohesion metric.
  template <typename T>
  void checkTypes(
    const std::string& path,
    const std::unordered_set<std::uint64_t>& typesFound,
    const std::unordered_map<std::uint64_t,std::string>& typeDefinitionPaths,
    std::unordered_multimap<std::string, std::uint64_t>& relationsFoundInFile,
    int& relationsInModule
  );

  std::vector<std::string> _inputPaths;
  std::string _modulesPath;
  std::unordered_set<model::FileId> _fileIdCache;
  std::unordered_map<model::CppAstNodeId, model::FileId> _astNodeIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
};
  
} // parser
} // cc

#endif // CC_PARSER_CPPMETRICSPARSER_H
