#ifndef CXXPARSER_DOCUMENTATIONCOMMENTFORMATTER_H
#define CXXPARSER_DOCUMENTATIONCOMMENTFORMATTER_H

#include <map>
#include <clang/AST/Comment.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <model/cxx/cppastnode.h>

#include "cxxparsesession.h"

namespace cc
{
namespace parser
{

/**
 * Converts a clang::comments::FullComment into an
 * HTML text
 */
class DocumentationCommentFormatter
{

public:

  /**
   * Converts a clang::comments::FullComment into an
   * HTML text
   *
   * @param SM the clang SourceManager
   * @param fc the comment to be formatted
   * @return the HTML-formatted string
   */
  std::string format(
      const clang::SourceManager &SM,
      clang::comments::FullComment *fc,
      std::map<std::string, unsigned long long> &params,
      CxxParseSession& session);

};

} // parser
} // cc

#endif //CXXPARSER_DOCUMENTATIONCOMMENTFORMATTER_H
