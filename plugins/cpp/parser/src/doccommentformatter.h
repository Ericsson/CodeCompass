#ifndef CC_PARSER_DOCCOMMENTFORMATTER_H
#define CC_PARSER_DOCCOMMENTFORMATTER_H

#include <clang/AST/Comment.h>

namespace cc
{
namespace parser
{

/**
 * Converts a clang::comments::FullComment into an HTML text.
 */
class DocCommentFormatter
{
public:

  /**
   * Converts a clang::comments::FullComment into an HTML text.
   *
   * @param fc The comment to be formatted.
   * @return the HTML-formatted string.
   */
  std::string format(
    clang::comments::FullComment* fc,
    const clang::ASTContext& ctx_);
};

} // parser
} // cc

#endif //CC_PARSER_DOCCOMMENTFORMATTER_H
