#include <set>
#include <algorithm>
#include <regex>

#include "util.h"

namespace
{

/**
 * This function will remove &* and *& from the parameter string.
 *
 * For example:
 *   x = &*&*y => x = y;
 */
void removeReferenceDereferencePairs(std::string& str_)
{
  if (!str_.empty())
  {
    std::regex pattern1("&\\*");
    std::regex pattern2("\\*&");

    if (str_[0] == '*')
      std::swap(pattern1, pattern2);

    str_ = std::regex_replace(str_, pattern1, "");
    str_ = std::regex_replace(str_, pattern2, "");
  }
}

}

namespace cc
{
namespace util
{

bool isDirectPointsTo(const model::CppPointerAnalysis& stmt_)
{
  return (stmt_.lhs.options & model::CppPointerAnalysis::Options::Reference) ||
    stmt_.rhs.options & (
      model::CppPointerAnalysis::Options::NullPtr   |
      model::CppPointerAnalysis::Options::HeapObj   |
      model::CppPointerAnalysis::Options::Undefined |
      model::CppPointerAnalysis::Options::Literal   |
      model::CppPointerAnalysis::Options::InitList  |
      model::CppPointerAnalysis::Options::Return    |
      model::CppPointerAnalysis::Options::FunctionCall);
}

bool isBaseConstraint(const model::CppPointerAnalysis& stmt_)
{
  return stmt_.lhs.operators.empty() &&
      ((!stmt_.rhs.operators.empty() && stmt_.rhs.operators == "&") ||
         isDirectPointsTo(stmt_));
}

bool isSimpleConstraint(const model::CppPointerAnalysis& stmt_)
{
  return stmt_.lhs.operators.empty() && stmt_.rhs.operators.empty();
}

void preprocessComplexStatements(
  std::vector<model::CppPointerAnalysis>& stmts_)
{
  for (model::CppPointerAnalysis& stmt : stmts_)
  {
    removeReferenceDereferencePairs(stmt.lhs.operators);
    removeReferenceDereferencePairs(stmt.rhs.operators);
  }

}

}
}
