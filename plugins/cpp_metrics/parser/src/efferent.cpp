#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>
#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>

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
    std::set<std::uint64_t> memberTypes;
    for (const model::CppRecord& type
      : _ctx.db->query<model::CppRecord>())
    {
      // Skip types that were included from external libraries.
      //if (!cc::util::isRootedUnderAnyOf(_inputPaths, type.))
        //continue;

      memberTypes.clear();
      for (const model::CppMemberType& mem : _ctx.db->query<model::CppMemberType>(
        MemTypeQuery::typeHash == type.entityHash &&
          MemTypeQuery::kind == model::CppMemberType::Kind::Field))
      {
        memberTypes.insert(mem.memberTypeHash);
      }

      model::CppAstNodeMetrics metric;
      metric.astNodeId = type.astNodeId;
      metric.type = model::CppAstNodeMetrics::Type::EFFERENT_TYPE;
      metric.value = memberTypes.size();
      _ctx.db->persist(metric);
    }
  });
}

} // parser
} // cc
