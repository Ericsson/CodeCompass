#include <cppmetricsparser/cppmetricsparser.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/odbtransaction.h>

#include <memory>

namespace cc
{
namespace parser
{

CppMetricsParser::CppMetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}

bool CppMetricsParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".dummy";
}

void CppMetricsParser::functionParameters()
{
  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::CppFunction func
      : _ctx.db->query<model::CppFunction>())
    {
      model::CppAstNodeMetrics funcParams;
      funcParams.astNodeId = func.astNodeId;
      funcParams.type = model::CppAstNodeMetrics::Type::PARAMETER_COUNT;
      funcParams.value = func.parameters.size();
      _ctx.db->persist(funcParams);
    }
  });
}

bool CppMetricsParser::parse()
{
  // Function parameter number metric.
  functionParameters();

  return true;
}

CppMetricsParser::~CppMetricsParser()
{
}

/* These two methods are used by the plugin manager to allow dynamic loading
   of CodeCompass Parser plugins. Clang (>= version 6.0) gives a warning that
   these C-linkage specified methods return types that are not proper from a
   C code.

   These codes are NOT to be called from any C code. The C linkage is used to
   turn off the name mangling so that the dynamic loader can easily find the
   symbol table needed to set the plugin up.
*/
// When writing a plugin, please do NOT copy this notice to your code.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("C++ Metrics Plugin");

    return description;
  }

  std::shared_ptr<CppMetricsParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CppMetricsParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
