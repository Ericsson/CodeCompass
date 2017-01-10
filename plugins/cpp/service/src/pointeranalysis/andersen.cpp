#include <chrono>

#include <util/logutil.h>

#include "util.h"
#include "andersen.h"

namespace cc
{
namespace service
{
namespace language
{

std::vector<model::CppPointerAnalysis> Andersen::init(
  const std::vector<model::CppPointerAnalysis>& statements_)
{
  std::vector<model::CppPointerAnalysis> complexStmts;
  std::vector<model::CppPointerAnalysis> simpleStmts;

  // Process statements which fit on base constraint and collects statements
  // which fit on simple and complex constraints.
  for (const model::CppPointerAnalysis& state : statements_)
  {
    if (util::isBaseConstraint(state))
      _PT[state.lhs].insert(state.rhs);
//    else if (util::isSimpleConstraint(state))
//      simpleStmts.push_back(state);
    else
      complexStmts.push_back(state);
  }

  //--- Process simple constraints ---//

  for (const model::CppPointerAnalysis& state : simpleStmts)
    _PT[state.lhs].insert(_PT[state.rhs].begin(), _PT[state.rhs].end());

  //--- Remove *&/&* from operators ---//

  util::preprocessComplexStatements(complexStmts);

  return complexStmts;
}

Andersen::PointsToSet Andersen::run(
  const std::vector<model::CppPointerAnalysis>& statements_)
{
  LOG(debug) << "Start Andersen style points-to algorithm.";

  std::chrono::steady_clock::time_point begin =
    std::chrono::steady_clock::now();

  //--- Initalize the algorithm ---//

  std::vector<model::CppPointerAnalysis> complexStmts = init(statements_);

  //--- Run algorithm on the complex statements ---//

  LOG(debug) << "Complex statements size: " << complexStmts.size();
  bool changed = !complexStmts.empty();
  while (changed)
  {
    changed = false;
    for (const model::CppPointerAnalysis& state : complexStmts)
    {
      for (const model::CppPointerAnalysis::StmtSide& lhs :evalLHS(state.lhs))
      {
        for (const model::CppPointerAnalysis::StmtSide& rhs : evalRHS(state.rhs))
        {
          if (!changed && _PT[lhs].find(rhs) == _PT[lhs].end())
            changed = true;
          _PT[lhs].insert(rhs);
        }
      }
    }
  };

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  LOG(warning)
    << "Andersen style points-to algorithm has been finished:\n"
    << "\t- time(ms): " << std::chrono::duration_cast<
       std::chrono::milliseconds>(end - begin).count();

  return _PT;
}

std::set<model::CppPointerAnalysis::StmtSide> Andersen::evalLHS(
  const model::CppPointerAnalysis::StmtSide& lhs_)
{
  if (lhs_.operators.empty())
    return {lhs_};
  else
  {
    std::set<model::CppPointerAnalysis::StmtSide> ret;
    std::string operators = lhs_.operators.substr(0, lhs_.operators.size() - 1);
    for (const model::CppPointerAnalysis::StmtSide& e : _PT[lhs_])
    {
      for (const model::CppPointerAnalysis::StmtSide& stmtSide : evalLHS(
        model::CppPointerAnalysis::StmtSide{e.mangledNameHash, operators,
        e.options}))
      {
        ret.insert(stmtSide);
      }
    }
    return ret;
  }
}

std::set<model::CppPointerAnalysis::StmtSide> Andersen::evalRHS(
  const model::CppPointerAnalysis::StmtSide& rhs_)
{
  if (rhs_.operators.empty())
  {
    return _PT[rhs_];
  }
  else if (rhs_.operators.back() == '&')
  {
    std::string operators = rhs_.operators.substr(0, rhs_.operators.size() - 1);

    if (operators.empty())
      return {rhs_};

    return evalRHS(model::CppPointerAnalysis::StmtSide{
      rhs_.mangledNameHash, operators, rhs_.options});
  }
  else
  {
    std::set<model::CppPointerAnalysis::StmtSide> ret;
    std::string operators = rhs_.operators.substr(0, rhs_.operators.size() - 1);
    for (const model::CppPointerAnalysis::StmtSide& e : _PT[rhs_])
    {
      for (const model::CppPointerAnalysis::StmtSide& stmtSide : evalRHS(
        model::CppPointerAnalysis::StmtSide{e.mangledNameHash, operators,
        e.options}))
      {
        ret.insert(stmtSide);
      }
    }
    return ret;
  }
}

}
}
}
