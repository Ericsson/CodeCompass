#ifndef CC_UTIL_UTIL_H
#define CC_UTIL_UTIL_H

#include <string>

namespace cc
{
namespace util
{

/**
 * This function returns the string representation of the current time.
 */
std::string getCurrentDate();

/**
 * This function returns a range from the given text. The line and column
 * coordinates are counted from 1.
 */
std::string textRange(
  const std::string& text_,
  std::size_t startLine_, std::size_t startCol_,
  std::size_t endLine_, std::size_t endCol_);

/**
 * This function escapes a string using HTML escape characters.
 * @param str_ String which will be escaped.
 * @return Escaped HTML sequence.
 */
std::string escapeHtml(const std::string& str_);

inline const char* boolToString(bool b) {
  return b ? "true" : "false";
}

}
}

#endif
