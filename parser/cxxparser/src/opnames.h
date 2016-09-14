#ifndef CXXPARSER_OPNAMES_H
#define CXXPARSER_OPNAMES_H

#include <string>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"

namespace cc
{
namespace parser
{
  
std::string unop2str(clang::UnaryOperatorKind kind);
std::string binop2str(clang::BinaryOperatorKind kind);

} // parser
} // cc

#endif //CXXPARSER_OPNAMES_H