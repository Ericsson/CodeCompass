#include <deque>
#include <chrono>

#include <util/logutil.h>

#include "pointeranalysisstmtcollector.h"

namespace
{

/**
 * This function return true if a statement has to be skipped.
 */
bool skipStmt(
  const cc::model::CppPointerAnalysis& stmt_,
  const cc::model::CppPointerAnalysis::StmtSide& current_)
{
  return current_.mangledNameHash == stmt_.rhs.mangledNameHash &&
    (current_.options & cc::model::CppPointerAnalysis::Options::Return ||
     current_.options & cc::model::CppPointerAnalysis::Options::FunctionCall);
}

}

namespace cc
{
namespace service
{

PointerAnalysisStmtCollector::PointerAnalysisStmtCollector(
  std::shared_ptr<odb::database> db_) : _db(db_), _transaction(db_)
{
}

std::vector<model::CppPointerAnalysis> PointerAnalysisStmtCollector::collect(
  const std::uint64_t& start_)
{
  LOG(debug) << "Start collecting pointer analysis statements.";

  std::chrono::steady_clock::time_point begin =
    std::chrono::steady_clock::now();

  std::vector<model::CppPointerAnalysis> statements;
  std::unordered_set<std::uint64_t> skipVariable;

  std::deque<model::CppPointerAnalysis::StmtSide> q;
  q.push_back({start_, 0, ""});

  while (!q.empty())
  {
    model::CppPointerAnalysis::StmtSide current = q.front();

    _transaction([&, this](){
      PointerAnalysisResult stmts = _db->query<model::CppPointerAnalysis>(
        (PointerAnalysisQuery::lhs.mangledNameHash == current.mangledNameHash ||
        PointerAnalysisQuery::rhs.mangledNameHash == current.mangledNameHash));

      std::unordered_set<std::uint64_t> funcParams;
      for (model::CppPointerAnalysis& stmt : stmts)
      {
        if (skipStmt(stmt, current))
          continue;

        if (stmt.lhs.options & model::CppPointerAnalysis::Options::Param &&
            start_ != stmt.lhs.mangledNameHash)
        {
          if (!skipVariable.count(stmt.lhs.mangledNameHash))
            funcParams.insert(stmt.lhs.mangledNameHash);
          else
            continue;
        }

        if (std::find(statements.begin(), statements.end(), stmt) ==
            statements.end())
        {
          if (current.mangledNameHash == stmt.rhs.mangledNameHash)
            q.push_back(stmt.lhs);
          else
            q.push_back(stmt.rhs);

          statements.push_back(stmt);
        }
      }
      skipVariable.insert(funcParams.begin(), funcParams.end());
    });
    q.pop_front();
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  LOG(debug)
    << "Pointer analysis collector has been finished:\n"
    << "\t- collected statements: " << statements.size() << "\n"
    << "\t- time(ms): " << std::chrono::duration_cast<
       std::chrono::milliseconds>(end - begin).count();

  return statements;
}

} // service
} // cc
