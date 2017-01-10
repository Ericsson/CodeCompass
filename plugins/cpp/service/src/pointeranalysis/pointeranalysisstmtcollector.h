#ifndef CC_SERVICE_POINTERANALYSISSTMTCOLLECTOR_H_
#define CC_SERVICE_POINTERANALYSISSTMTCOLLECTOR_H_

#include <vector>

#include <model/cpppointeranalysis.h>
#include <model/cpppointeranalysis-odb.hxx>

#include <projectservice/projectservice.h>

namespace cc
{
namespace service
{

/**
 * Pointer analysis statement collector.
 */
class PointerAnalysisStmtCollector
{
public:
  PointerAnalysisStmtCollector(std::shared_ptr<odb::database> db_);

  /**
   * This function collects pointer analysis statements recursively from
   * database related to the `start_` parameter.
   * @param start_ Starting AST node mangled name hash where statements
   * collecting starts.
   * @return List of collected statements.
   */
  std::vector<model::CppPointerAnalysis> collect(const std::uint64_t& start_);

private:
  typedef typename odb::query<model::CppPointerAnalysis> PointerAnalysisQuery;
  typedef typename odb::result<model::CppPointerAnalysis> PointerAnalysisResult;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
};

} // service
} // cc

#endif // CC_SERVICE_POINTERANALYSISSTMTCOLLECTOR_H_
