#include <vector>

#include <model/cpppointeranalysis.h>

namespace cc
{
namespace util
{

/**
 * Return true if the statement satisfies the simple constraint condition.
 *
 * For example:
 * @code
 * x = &b;
 * @code
 */
bool isBaseConstraint(const model::CppPointerAnalysis& stmt_);

/**
 * Return true if the statement satisfies the simple constraint condition.
 *
 * For example:
 * @code
 * x = b;
 * @code
 */
bool isSimpleConstraint(const model::CppPointerAnalysis& stmt_);

/**
 * Return true if the statement left side points to directly to the right side
 * of the statement. Left side directly points to the right side if left side is
 * a reference to the right side or if the right side is a nullptr, undefined,
 * string literal, initialization list, function call or a heap object.
 */
bool isDirectPointsTo(const model::CppPointerAnalysis& stmt_);

/**
 * This function will remove `&*` and `*&` from the both side of statement
 * operators.
 *
 * For example:
 *   x = &*&*y => x = y;
 */
void preprocessComplexStatements(
  std::vector<model::CppPointerAnalysis>& stmts_);

}
}
