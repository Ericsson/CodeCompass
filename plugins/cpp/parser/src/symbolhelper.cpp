#include <cstring>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Index/USRGeneration.h>
#include <clang/Lex/Lexer.h>

#include <model/fileloc.h>
#include <model/fileloc-odb.hxx>
#include <util/logutil.h>

#include "symbolhelper.h"

namespace
{

/**
 * This function removes the first occurence of what_ in str_ if any.
 */
void removeSubstring(std::string& str_, const std::string& what_)
{
  std::size_t pos = str_.find(what_);
  if (pos != std::string::npos)
    str_.erase(pos, what_.size());
}

}

namespace cc
{
namespace parser
{

// TODO: Isn't it same as qt_.getTypePtr()?
const clang::Type* getStrippedType(const clang::QualType& qt_)
{
  const clang::Type* type = qt_.getTypePtrOrNull();

  while (type)
  {
    if (type->getAs<clang::TypedefType>())
      break;

    if (const clang::ParenType* parenType = type->getAs<clang::ParenType>())
      type = parenType->getInnerType().getTypePtrOrNull();
    else if (type->isReferenceType() || type->isPointerType())
      type = type->getPointeeType().getTypePtrOrNull();
    else if (const clang::ArrayType* arrayType =
             llvm::dyn_cast<clang::ArrayType>(type))
      type = arrayType->getElementType().getTypePtrOrNull();
    else
      break;
  }

  return type;
}

std::string getUSR(const clang::NamedDecl* nd_)
{
  // Function generateUSRForDecl() returns a boolean value which indicates
  // whether the generated USR should be ignored or not. It is easily seen in
  // the implementation when this boolean is true:
  // https://clang.llvm.org/doxygen/USRGeneration_8cpp_source.html
  // Just search for "IgnoreResults" variable and see where it's set true.
  // Some examples are the AST node for linkage specifiers (i.e. extern "C"),
  // the "using" directive, and some unnamed symbols, like the parameter of
  // this function pointer: void (*f)(int);
  // This last one is important from CodeCompass point of view: we store a
  // CppEntity in the database for the parameter variable of this function
  // declaration, however, according to Clang documentation the generated USR
  // for this node is supposed to be ignored. I can't see why the generation of
  // this USR can't be finished. Maybe it is not possible to distinguish this
  // node from another one we don't know about. Anyway, the generated USR is
  // good enough for us to identify this unnamed variable, because according to
  // our experiences it is quite unique and the variable can't be clicked on
  // the GUI (since it has no name to click on).
  // So far I have met only one case when an USR which is supposed to be
  // ignored is not good enough: the unnamed parameter of a compiler-generated
  // copy/move constructor and assignment operator. The USR of these parameters
  // is the same as the USR of these functions themselves.
  // So skipping the return value of generateUSRForDecl() is a conscious
  // choice.
  llvm::SmallVector<char, 64> usr;
  clang::index::generateUSRForDecl(nd_, usr);
  char* data = usr.data();
  return std::string(data, data + usr.size());
}

std::string getUSR(const clang::QualType& qt_, clang::ASTContext& ctx_)
{
  const clang::Type* type = qt_.getTypePtr();

  if (const clang::TypedefType* td = type->getAs<clang::TypedefType>())
  {
    if (const clang::TypedefNameDecl* tDecl = td->getDecl())
      return getUSR(tDecl);
  }
  else if (const clang::TagDecl* tDecl = type->getAsTagDecl())
    return getUSR(tDecl);

  // This is supposed to run only for base types.
  llvm::SmallVector<char, 64> usr;
  clang::index::generateUSRForType(qt_, ctx_, usr);
  char* data = usr.data();
  return std::string(data, data + usr.size());
}

bool isFunction(const clang::Type* type_)
{
  while (type_)
    if (type_->isFunctionType() ||
        type_->isFunctionPointerType() ||
        type_->isVoidPointerType())
      return true;
    else if (auto* parenType = type_->getAs<clang::ParenType>())
      type_ = parenType->getInnerType().getTypePtrOrNull();
    else if (auto* arrayType = llvm::dyn_cast<clang::ArrayType>(type_))
      type_ = arrayType->getElementType().getTypePtrOrNull();
    else if (type_->isPointerType() || type_->isReferenceType())
      type_ = type_->getPointeeType().getTypePtrOrNull();
    else
      return false;

  return false;
}

// This function is from an earlier version of Clang code base. I couldn't find
// this implementation in later versions.
std::string getSignature(const clang::FunctionDecl* fn_)
{
  if (!fn_)
    return "";

  std::string signature;

  if (!llvm::isa<clang::CXXConstructorDecl>(fn_) &&
      !llvm::isa<clang::CXXDestructorDecl>(fn_) &&
      !llvm::isa<clang::CXXConversionDecl>(fn_))
  {
    std::string returnType = fn_->getReturnType().getAsString();
    removeSubstring(returnType, "struct ");
    removeSubstring(returnType, "class ");
    signature.append(returnType).append(" ");
  }

  signature.append(fn_->getQualifiedNameAsString()).append("(");

  for (int i = 0, paramsCount = fn_->getNumParams(); i < paramsCount; ++i)
  {
    if (i)
      signature.append(", ");

    std::string paramType = fn_->getParamDecl(i)->getType().getAsString();
    removeSubstring(paramType, "struct ");
    removeSubstring(paramType, "class ");

    signature.append(paramType);
  }

  if (fn_->isVariadic())
    signature.append(", ...");

  signature.append(")");

  const auto *targetT
    = llvm::dyn_cast_or_null<clang::FunctionType>(fn_->getType().getTypePtr());

  if (!targetT || !llvm::isa<clang::CXXMethodDecl>(fn_))
    return signature;

  if (targetT->isConst())
    signature.append(" const");
  if (targetT->isVolatile())
    signature.append(" volatile");
  if (targetT->isRestrict())
    signature.append(" restrict");

  if (const auto *targetPT = llvm::dyn_cast_or_null<clang::FunctionProtoType>(
    fn_->getType().getTypePtr()))
  {
    switch (targetPT->getRefQualifier())
    {
      case clang::RQ_LValue: signature.append(" &");  break;
      case clang::RQ_RValue: signature.append(" &&"); break;
      default: break;
    }
  }

  return signature;
}

std::string getDeclPartAsString(
  const clang::SourceManager& srcMgr_,
  const clang::TagDecl* td_)
{
  return td_->isThisDeclarationADefinition()
    ? getSourceText(
        srcMgr_,
        td_->getOuterLocStart(),
        td_->getBraceRange().getBegin())
    : getSourceText(
        srcMgr_,
        td_->getOuterLocStart(),
        td_->getEndLoc(),
        true);
}

std::string getSourceText(
  const clang::SourceManager& srcMgr_,
  clang::SourceLocation from_,
  clang::SourceLocation to_,
  bool inclusiveEnd_)
{
  clang::LangOptions lopt;

  if (inclusiveEnd_)
    to_ = clang::Lexer::getLocForEndOfToken(to_, 0, srcMgr_, lopt);

  clang::CharSourceRange range = clang::Lexer::getAsCharRange(
    clang::SourceRange(from_, to_),
    srcMgr_,
    lopt);

  return clang::Lexer::getSourceText(range, srcMgr_, lopt).str();
}

}
}
