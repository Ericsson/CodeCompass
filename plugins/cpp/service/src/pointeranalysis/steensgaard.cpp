#include <chrono>

#include <util/logutil.h>

#include "util.h"
#include "steensgaard.h"

namespace cc
{
namespace service
{
namespace language
{

Steensgaard::Steensgaard() :
  _ds(boost::make_assoc_property_map(_rank),
      boost::make_assoc_property_map(_parent))
{
}

void Steensgaard::process(const TypeNodePtr& n_, const TypeNodePtr& m_)
{
  if (n_ && m_)
  {
    if (n_->pointsTo && n_->pointsTo != m_)
      merge(m_, n_->pointsTo);
    else
      n_->pointsTo = m_;
  }
}

std::vector<model::CppPointerAnalysis> Steensgaard::init(
  const std::vector<model::CppPointerAnalysis>& statements_)
{
  std::vector<model::CppPointerAnalysis> complexStmts;

  //--- Collect unique variables ---//

  std::set<model::CppPointerAnalysis::StmtSide> variables;
  for (const model::CppPointerAnalysis& statement : statements_)
  {
    variables.insert(statement.lhs);
    variables.insert(statement.rhs);
  }

  //--- Create a type set for each variable ---//

  for (const model::CppPointerAnalysis::StmtSide& var : variables)
  {
    _type[var] = std::make_shared<TypeNode>(var);
    _ds.make_set(_type[var]);
  }

  for (const model::CppPointerAnalysis& state : statements_)
  {
    if (util::isBaseConstraint(state))
    {
      TypeNodePtr n = _type[state.lhs];
      TypeNodePtr m = _type[state.rhs];

      process(n, m);
    }
    else
      complexStmts.push_back(state);
  }

  //--- Remove *&/&* from operators ---//

  util::preprocessComplexStatements(complexStmts);

  return complexStmts;
}

std::map<model::CppPointerAnalysis::StmtSide, Steensgaard::TypeNodePtr>
Steensgaard::run(const std::vector<model::CppPointerAnalysis>& statements_)
{
  LOG(debug) << "Start Steensgaard style points-to algorithm.";

  std::chrono::steady_clock::time_point begin =
    std::chrono::steady_clock::now();

  //--- Initalize the algorithm ---//

  std::vector<model::CppPointerAnalysis> complexStmts = init(statements_);

  //--- Run algorithm on the complex statements ---//

  for (const model::CppPointerAnalysis& statement : complexStmts)
  {
    // 1. Find the node n that represents the variable or variables
    // being assigned to.
    TypeNodePtr n = evalLhs(statement.lhs);

    // 2. Find the node m that represents the value being assigned.
    TypeNodePtr m = evalRhs(statement.rhs);

    process(n, m);
  }

  for (const auto& key : _type)
  {
    TypeNodePtr v = key.second;
    v->value = _ds.find_set(v)->value;
    if (v->pointsTo)
      v->pointsTo->value = _ds.find_set(v->pointsTo)->value;
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  LOG(warning)
    << "Steensgaard style points-to algorithm has been finished:\n"
    << "\t- time(ms): " << std::chrono::duration_cast<
       std::chrono::milliseconds>(end - begin).count();

  return _type;
}

Steensgaard::TypeNodePtr Steensgaard::evalLhs(
  const model::CppPointerAnalysis::StmtSide& lhs_)
{
  if (lhs_.operators.empty())
    return _type.at(lhs_);
  else
  {
    std::string operators = lhs_.operators.substr(0, lhs_.operators.size() - 1);
    TypeNodePtr t = _type[lhs_]->getTarget();

    if (!t)
      return  nullptr;

    return evalLhs({t->value.mangledNameHash, lhs_.options, operators});
  }
}

Steensgaard::TypeNodePtr Steensgaard::evalRhs(
  const model::CppPointerAnalysis::StmtSide& rhs_)
{
  if (rhs_.operators.empty())
  {
    TypeNodePtr t = _type.at(rhs_);
    return t->getTarget();
  }
  else if (rhs_.operators.back() == '&')
  {
    std::string operators = rhs_.operators.substr(0, rhs_.operators.size() - 1);

    if (operators.empty())
      return _type.at(rhs_);

    return evalRhs({rhs_.mangledNameHash, rhs_.options, operators});
  }
  else
  {
    std::string operators = rhs_.operators.substr(0, rhs_.operators.size() - 1);
    TypeNodePtr t = _type[rhs_]->getTarget();

    if (!t)
      return  nullptr;

    return evalRhs({t->value.mangledNameHash, rhs_.options, operators});
  }
}

Steensgaard::TypeNodePtr Steensgaard::merge(
  const TypeNodePtr& t1_,
  const TypeNodePtr& t2_)
{
  if (!t1_) return t2_;
  if (!t2_ || _ds.find_set(t1_) == _ds.find_set(t2_)) return t1_;

  _ds.union_set(t1_, t2_);

  TypeNodePtr u = _ds.find_set(t1_);
  u->pointsTo = merge(t1_->pointsTo, t2_->pointsTo);

  return u;
}

}
}
}
