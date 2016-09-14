#ifndef CXXPARSER_SYMBOLHELPER_H
#define CXXPARSER_SYMBOLHELPER_H

#include <string>
#include <map>

#include <clang/AST/Mangle.h>
#include <clang/AST/GlobalDecl.h>
#include <clang/AST/ASTContext.h>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>

#include <model/workspace.h>
#include <model/cxx/cppastnode.h>
#include <model/file.h>

namespace cc
{
namespace parser
{

class SymbolHelper
{
public:
  SymbolHelper(clang::ASTContext& astctx_) :
    _mgctx(astctx_.createMangleContext()) {}

  std::string getMangledName(clang::GlobalDecl gd_) const;

  std::string getMangledNameOfLocal(clang::GlobalDecl gd_, model::CppAstNodePtr node) const;
  std::string getMangledNameOfField(clang::NamedDecl* nd_, model::CppAstNodePtr node) const;
  std::string getMangledNameOfType(clang::RecordDecl* nd_, model::CppAstNodePtr node) const;
  std::string getMangledNameOfEnum(clang::EnumDecl* ed_, model::CppAstNodePtr node) const;
  std::string getMangledNameOfTypedef(clang::TypedefNameDecl* ed_, model::CppAstNodePtr node) const;
  std::string getMangledNameOfEnumConstant(clang::EnumConstantDecl* ecd_, model::CppAstNodePtr node) const;

  std::string getSuffixFromNode(const model::CppAstNodePtr& node) const;

  /**
   * Retuns the type name without any namespace.
   *
   * @param rd_ a record declaration.
   * @return the name of the type.
   */
  std::string getTypeName(const clang::RecordDecl* rd_) const;

  /**
   * Retuns the type name without any namespace.
   *
   * @param rd_ a record declaration.
   * @return the full qualified type name.
   */
  std::string getTypeQualifiedName(const clang::RecordDecl* rd_) const;

private:

  std::unique_ptr<clang::MangleContext> _mgctx;
};

} // parser
} // cc

#endif
