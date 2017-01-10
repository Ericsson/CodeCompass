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
  std::set<model::CppPointerAnalysis::Options> rhsOpt = stmt_.rhs.options;
  return model::isReference(stmt_.lhs) ||
    std::any_of(rhsOpt.begin(), rhsOpt.end(), [] (
      model::CppPointerAnalysis::Options opt_)
    {
      return
        opt_ == model::CppPointerAnalysis::Options::NullPtr ||
        opt_ == model::CppPointerAnalysis::Options::HeapObj ||
        opt_ == model::CppPointerAnalysis::Options::Undefined ||
        opt_ == model::CppPointerAnalysis::Options::Literal ||
        opt_ == model::CppPointerAnalysis::Options::InitList ||
        opt_ == model::CppPointerAnalysis::Options::Return ||
        opt_ == model::CppPointerAnalysis::Options::FunctionCall;
    });
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
