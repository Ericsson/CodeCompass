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

#include <util/dbutil.h>
#include <util/parserutil.h>
#include <util/threadpool.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{

template<typename TTask>
class MetricsTasks
{
public:
  typedef typename std::vector<TTask>::const_iterator TTaskIter;

  const TTaskIter& begin() const { return _begin; }
  const TTaskIter& end() const { return _end; }
  std::size_t size() const { return _size; }

  MetricsTasks(
    const TTaskIter& begin_,
    const TTaskIter& end_,
    std::size_t size_
  ) :
    _begin(begin_),
    _end(end_),
    _size(size_)
  {}

private:
  TTaskIter _begin;
  TTaskIter _end;
  std::size_t _size;
};


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
    return cc::util::getFilterPathsQuery<TQueryParam>(
      _inputPaths.begin(), _inputPaths.end());
  }

  /// @brief Calculates a metric by querying all objects of the
  /// specified parameter type and passing them one-by-one to the
  /// specified worker function on parallel threads.
  /// This call blocks the caller thread until all workers are finished.
  /// @tparam TQueryParam The type of parameters to query.
  /// @param name_ The name of the metric (for progress logging).
  /// @param partitions_ The number of jobs to partition the query into.
  /// @param query_ A filter query for retrieving only
  /// the eligible parameters for which a worker should be spawned.
  /// @param worker_ The logic of the worker thread.
  template<typename TQueryParam>
  void parallelCalcMetric(
    const char* name_,
    std::size_t partitions_,
    const odb::query<TQueryParam>& query_,
    const std::function<void(const MetricsTasks<TQueryParam>&)>& worker_)
  {
    typedef MetricsTasks<TQueryParam> TMetricsTasks;
    typedef typename TMetricsTasks::TTaskIter TTaskIter;
    typedef std::pair<std::size_t, TMetricsTasks> TJobParam;

    // Define the thread pool and job wrapper function.
    LOG(info) << name_ << " : Collecting jobs from database...";
    std::unique_ptr<util::JobQueueThreadPool<TJobParam>> pool =
      util::make_thread_pool<TJobParam>(_threadCount,
        [&](const TJobParam& job)
      {
        LOG(info) << '(' << job.first << '/' << partitions_
          << ") " << name_;
        worker_(job.second);
      });

    // Cache the results of the query that will be dispatched to workers.
    std::vector<TQueryParam> tasks;
    util::OdbTransaction {_ctx.db} ([&, this]
    {
      // Storing the result directly and then calling odb::result<>::cache()
      // on it does not work: odb::result<>::size() will always throw
      // odb::result_not_cached. As of writing, this is a limitation of SQLite.
      // So we fall back to the old-fashioned way: std::vector<> in memory.
      for (const TQueryParam& param : _ctx.db->query<TQueryParam>(query_))
        tasks.emplace_back(param);
    });

    // Ensure that all workers receive at least one task.
    std::size_t taskCount = tasks.size();
    if (partitions_ > taskCount)
      partitions_ = taskCount;

    // Dispatch jobs to workers in discrete packets.
    LOG(info) << name_ << " : Dispatching jobs on "
      << _threadCount << " thread(s)...";
    std::size_t prev = 0;
    TTaskIter it_prev = tasks.cbegin();

    std::size_t i = 0;
    while (i < partitions_)
    {
      std::size_t next = taskCount * ++i / partitions_;
      std::size_t size = next - prev;
      TTaskIter it_next = it_prev;
      std::advance(it_next, size);

      pool->enqueue(TJobParam(i, TMetricsTasks(it_prev, it_next, size)));

      prev = next;
      it_prev = it_next;
    }

    // Await the termination of all workers.
    pool->wait();
    LOG(info) << name_ << " : Calculation finished.";
  }

  /// @brief Calculates a metric by querying all objects of the
  /// specified parameter type and passing them one-by-one to the
  /// specified worker function on parallel threads.
  /// This call blocks the caller thread until all workers are finished.
  /// @tparam TQueryParam The type of parameters to query.
  /// @param name_ The name of the metric (for progress logging).
  /// @param partitions_ The number of jobs to partition the query into.
  /// @param worker_ The logic of the worker thread.
  template<typename TQueryParam>
  void parallelCalcMetric(
    const char* name_,
    std::size_t partitions_,
    const std::function<void(const MetricsTasks<TQueryParam>&)>& worker_)
  {
    parallelCalcMetric<TQueryParam>(
      name_,
      partitions_,
      odb::query<TQueryParam>(),
      worker_);
  }


  int _threadCount;
  std::vector<std::string> _inputPaths;
  std::unordered_set<model::FileId> _fileIdCache;
  std::unordered_map<model::CppAstNodeId, model::FileId> _astNodeIdCache;

  static const int functionParamsPartitionMultiplier = 5;
  static const int functionMcCabePartitionMultiplier = 5;
  static const int functionBumpyRoadPartitionMultiplier = 5;
  static const int lackOfCohesionPartitionMultiplier = 25;
};
  
} // parser
} // cc

#endif // CC_PARSER_CPPMETRICSPARSER_H
