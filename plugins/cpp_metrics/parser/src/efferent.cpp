#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>
#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>

#include <util/filesystem.h>
#include <util/odbtransaction.h>

#include "efferent.h"

namespace cc
{
namespace parser
{

EfferentCoupling::EfferentCoupling(
  ParserContext& ctx_,
  std::vector<std::string> inputPaths_)
  : _ctx(ctx_), _inputPaths(inputPaths_)
{
}

void EfferentCoupling::efferentTypeLevel()
{
  typedef odb::query<cc::model::CppMemberType> MemTypeQuery;

  util::OdbTransaction{_ctx.db}([&, this]
  {
    for (const model::CppRecord& type
      : _ctx.db->query<model::CppRecord>())
    {
      // Skip types that were included from external libraries.
      //if (!cc::util::isRootedUnderAnyOf(_inputPaths, type.))
        //continue;

      auto count = _ctx.db->query_value<model::CppMemberTypeCount>(
        MemTypeQuery::typeHash == type.entityHash &&
        MemTypeQuery::kind == model::CppMemberType::Kind::Field).count;

      model::CppAstNodeMetrics metric;
      metric.astNodeId = type.astNodeId;
      metric.type = model::CppAstNodeMetrics::Type::EFFERENT_TYPE;
      metric.value = count;
      _ctx.db->persist(metric);
    }
  });
}

} // parser
} // cc
