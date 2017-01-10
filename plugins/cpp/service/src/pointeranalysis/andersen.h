#ifndef CC_SERVICE_LANGUAGE_ANDERSEN_H
#define CC_SERVICE_LANGUAGE_ANDERSEN_H

#include <set>
#include <map>
#include <vector>

#include <model/cpppointeranalysis.h>

namespace cc
{
namespace service
{
namespace language
{

/**
 * Andersen points-to analysis.
 *
 * @note Andersen points-to algorithm is a flow-insensitive points-to algorithm.
 * It views pointer assignments as subset constraints and use these constraints
 * to propagate points-to information. Creates a point-to set for each variable.
 */
class Andersen
{
public:
  /**
   * Point-to set for each variable.
   * E.g.: pts(a) = {b} means that `a` may point to `b` in the entire program.
   */
  typedef std::map<model::CppPointerAnalysis::StmtSide,
    std::set<model::CppPointerAnalysis::StmtSide>> PointsToSet;

  /**
   * Run Andersen points-to algorithm on the statements.
   *
   * @param statements_ Collected statements.
   * @return One points-to set per pointer variable.
   */
  PointsToSet run(const std::vector<model::CppPointerAnalysis>& statements_);

private:
  /**
   * This function will initialize points to sets using simple constraints.
   * @param statements_ Collected statements.
   * @return Statements which have complex constraints.
   */
  std::vector<model::CppPointerAnalysis> init(
    const std::vector<model::CppPointerAnalysis>& statements_);

  /**
   * This function evaluates the left side of a statement.
   *
   * @param lhs_ Left side of a statement.
   * @return Point-to set.
   */
  std::set<model::CppPointerAnalysis::StmtSide> evalLHS(
    const model::CppPointerAnalysis::StmtSide& lhs_);

  /**
   * This function recursively evaluates the right side of a statement.
   *
   * @param rhs_ Right side of a statement.
   * @return Point-to set.
   */
  std::set<model::CppPointerAnalysis::StmtSide> evalRHS(
    const model::CppPointerAnalysis::StmtSide& rhs_);

  /**
   * Point-to set information for each variable in the statements.
   */
  PointsToSet _PT;
};

} // language
} // service
} // cc

#endif // CC_SERVICE_LANGUAGE_ANDERSEN_H
