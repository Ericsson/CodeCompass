#ifndef CC_PARSER_SYMBOLHELPER_H
#define CC_PARSER_SYMBOLHELPER_H

#include <string>

#include <clang/AST/Decl.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/Type.h>

#include <model/fileloc.h>

namespace cc
{
namespace parser
{

const clang::Type* getStrippedType(const clang::QualType& qt_);

std::string getUSR(const clang::NamedDecl* nd_);
std::string getUSR(const clang::QualType& qt_, clang::ASTContext& ctx_);

bool isFunction(const clang::Type* type_);

std::string getSignature(const clang::FunctionDecl* fn_);

/**
 * Returns the source range of the prototype part of a tag declaration. If it's
 * a definition then this range ends before the opening curly brace, otherwise
 * at the end of the declaration.
 * For example:
 * template <typename> struct S { int i; };
 * ^                          ^
 * enum Color;
 * ^        ^
 */
std::string getDeclPartAsString(
  const clang::SourceManager& srcMgr_,
  const clang::TagDecl* td_);

/**
 * This function returns the source code fragment between the given source
 * locations. inclusiveEnd determines whether the end of the range is
 * inclusive, i.e. the token starting at "to" is also the part of returned code
 * text.
 */
std::string getSourceText(
  const clang::SourceManager& srcMgr_,
  clang::SourceLocation from_,
  clang::SourceLocation to_,
  bool inclusiveEnd_ = false);

}
}

#endif
