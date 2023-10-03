#ifndef CC_PARSER_DUMMYPARSER_H
#define CC_PARSER_DUMMYPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>

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
  virtual bool parse() override;

private:
  bool accept(const std::string& path_);

  void functionParameters();

  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_CPPMETRICSPARSER_H
