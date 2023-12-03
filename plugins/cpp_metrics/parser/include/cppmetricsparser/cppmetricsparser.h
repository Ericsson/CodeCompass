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
  void functionParameters();
  void functionMcCabe();
  void lackOfCohesion();
  
  // Checks if the given (canonical) path begins with any of the input paths
  // specified for the metrics parser via command line arguments on startup.
  bool isInInputPath(const std::string& path_) const;
  std::vector<std::string> _inputPaths;
  
  std::unordered_set<model::FileId> _fileIdCache;
  std::unordered_map<model::CppAstNodeId, model::FileId> _astNodeIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
};
  
} // parser
} // cc

#endif // CC_PARSER_CPPMETRICSPARSER_H
