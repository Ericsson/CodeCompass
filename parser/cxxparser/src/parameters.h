#ifndef __CC_PARSER_CXXPARSER_PARAMETERS_H_
#define __CC_PARSER_CXXPARSER_PARAMETERS_H_

#include <vector>
#include <string>

namespace clang
{
  class DiagnosticsEngine;
}

namespace cc
{
namespace parser
{

/**
 * A vector of compiler arguments.
 */
using CompilerArgments = std::vector<std::string>;

/**
 * Processes a full C/C++ compilation command line. The result is a vectort of
 * command line arguments for each translation unit in the command line. If
 * something fails, the result is an empty vector.
 *
 * @param options_ a full C/C++ command line
 * @param diag_ a diag engine for reporting errors
 * @return a vector of arguments for each tr unit or empty vector on error
 */
std::vector<CompilerArgments> processCommandLine(
  CompilerArgments                options_,
  clang::DiagnosticsEngine&       diag_);

} // parser
} // cc

#endif // __CC_PARSER_CXXPARSER_PARAMETERS_H_
