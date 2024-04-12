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
#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{

/// @brief Constructs an ODB query that you can use to filter only
/// the database records of the given parameter type whose path
/// is rooted under any of the specified filter paths.
/// @tparam TQueryParam The type of database records to query.
/// This type must represent an ODB view that has access to
/// (i.e. is also joined with) the File table.
/// @tparam TIter The iterator type of the filter paths.
/// @tparam TSentinel The type of the end of the filter paths.
/// @param begin_ The iterator referring to the first filter path.
/// @param end_ The sentinel for the end of the filter paths.
/// @return A query containing the disjunction of filters.
template<typename TQueryParam, typename TIter, typename TSentinel>
odb::query<TQueryParam> getFilterPathsQuery(
  TIter begin_,
  const TSentinel& end_)
{
  typedef typename odb::query<TQueryParam>::query_columns QParam;
  const auto& QParamPath = QParam::File::path;
  constexpr char ODBWildcard = '%';

  assert(begin_ != end_ && "At least one filter path must be provided.");
  odb::query<TQueryParam> query = QParamPath.like(*begin_ + ODBWildcard);
  while (++begin_ != end_)
    query = query || QParamPath.like(*begin_ + ODBWildcard);
  return query;
}


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
  // Calculate the lack of cohesion between member variables
  // and member functions for every type.
  void lackOfCohesion();


  /// @brief Constructs an ODB query that you can use to filter only
  /// the database records of the given parameter type whose path
  /// is rooted under any of this parser's input paths.
  /// @tparam TQueryParam The type of database records to query.
  /// This type must represent an ODB view that has access to
  /// (i.e. is also joined with) the File table.
  /// @return A query containing the disjunction of filters.
  template<typename TQueryParam>
  odb::query<TQueryParam> getFilterPathsQuery() const
  {
    return cc::parser::getFilterPathsQuery<TQueryParam>(
      _inputPaths.begin(), _inputPaths.end());
  }

  /// @brief Calculates a metric by querying all objects of the
  /// specified parameter type and passing them one-by-one to the
  /// specified worker function on parallel threads.
  /// This call blocks the caller thread until all workers are finished.
  /// @tparam TQueryParam The type of parameters to query.
  /// @param query_ A filter query for retrieving only
  /// the eligible parameters for which a worker should be spawned.
  /// @param worker_ The logic of the worker thread.
  template<typename TQueryParam>
  void parallelCalcMetric(
    const odb::query<TQueryParam>& query_,
    const std::function<void(const TQueryParam&)>& worker_)
  {
    std::unique_ptr<util::JobQueueThreadPool<TQueryParam>> pool =
      util::make_thread_pool<TQueryParam>(_threadCount, worker_);
    util::OdbTransaction {_ctx.db} ([&, this]
    {
      for (const TQueryParam& param : _ctx.db->query<TQueryParam>(query_))
        pool->enqueue(param);
    });
    pool->wait();
  }

  /// @brief Calculates a metric by querying all objects of the
  /// specified parameter type and passing them one-by-one to the
  /// specified worker function on parallel threads.
  /// This call blocks the caller thread until all workers are finished.
  /// @tparam TQueryParam The type of parameters to query.
  /// @param worker_ The logic of the worker thread.
  template<typename TQueryParam>
  void parallelCalcMetric(
    const std::function<void(const TQueryParam&)>& worker_)
  { parallelCalcMetric<TQueryParam>(odb::query<TQueryParam>(), worker_); }


  int _threadCount;
  std::vector<std::string> _inputPaths;
  std::unordered_set<model::FileId> _fileIdCache;
  std::unordered_map<model::CppAstNodeId, model::FileId> _astNodeIdCache;
};
  
} // parser
} // cc

#endif // CC_PARSER_CPPMETRICSPARSER_H
