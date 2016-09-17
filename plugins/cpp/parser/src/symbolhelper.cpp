#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/DeclCXX.h>

#include <model/fileloc.h>
#include <model/fileloc-odb.hxx>

#include "symbolhelper.h"

namespace
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

std::string getSuffixFromLoc(const cc::model::FileLoc& fileLoc_)
{
  if (!fileLoc_.file)
    return std::string();

  return 
      std::to_string(fileLoc_.file.object_id()) + ':'
    + std::to_string(fileLoc_.range.start.line) + ':'
    + std::to_string(fileLoc_.range.start.column);
}

}

namespace cc
{
namespace parser
{

std::string getMangledName(
  clang::MangleContext* mangleContext_,
  const clang::NamedDecl* nd_,
  const model::FileLoc& fileLoc_)
{
  if (llvm::isa<clang::VarDecl>(nd_))
  {
    std::string result = nd_->getQualifiedNameAsString();

    if (const clang::DeclContext* dc = nd_->getParentFunctionOrMethod())
    {
      result += ':' + getMangledName(
        mangleContext_,
        llvm::dyn_cast<clang::FunctionDecl>(dc));

      if (const clang::ParmVarDecl* pvd =
          llvm::dyn_cast<clang::ParmVarDecl>(nd_))
        result +=  ':' + std::to_string(pvd->getFunctionScopeIndex());
    }

    return result;
  }
  else if (llvm::isa<clang::NamespaceDecl>(nd_))
  {
    const clang::NamespaceDecl* na_ = llvm::dyn_cast<clang::NamespaceDecl>(nd_);
    return na_->isAnonymousNamespace()
      ? "anonymous-ns:" + std::to_string(fileLoc_.file.object_id())
      : nd_->getQualifiedNameAsString();
  }

  // TODO: For some reason some named decls don't have name.
  // Function parameters without name (e.g. compiler generated functions) don't
  // have a name, but we can generate them a unique name later.
  if (nd_->getNameAsString().empty() && !llvm::isa<clang::FunctionDecl>(nd_))
    return nd_->getQualifiedNameAsString();

  // No need to mangle C sources or declarations in extern "C".
  if (!mangleContext_->shouldMangleDeclName(nd_))
    return nd_->getQualifiedNameAsString();

  const clang::FunctionDecl* fd = llvm::dyn_cast<clang::FunctionDecl>(nd_);
  if (fd && llvm::isa<clang::FunctionProtoType>(
    fd->getType()->getAs<clang::FunctionType>()))
  {
    // FIXME: Workaround for a crash reported in #236
    // From Clang's code:
    // We should never mangle anything without a prototype.

    std::string str;
    llvm::raw_string_ostream out(str);

    if (const clang::CXXConstructorDecl* ctor
        = llvm::dyn_cast<clang::CXXConstructorDecl>(fd))
      mangleContext_->mangleCXXCtor(ctor, clang::Ctor_Complete, out);
    else if (const clang::CXXDestructorDecl* dtor
        = llvm::dyn_cast<clang::CXXDestructorDecl>(fd))
      mangleContext_->mangleCXXDtor(dtor, clang::Dtor_Complete, out);
    else
      mangleContext_->mangleName(fd, out);
    
    // TODO: The function's own mangled name should be enough. The suffix
    // shouldn't be necessary. The reason is
    // odb::details::shared_ptr::shared_ptr<X> where there is a template and a
    // non-template version, and their mangled name is the same.
    std::string suffix = getSuffixFromLoc(fileLoc_);
    return out.str() + (suffix.empty() ? "" : ':' + suffix);
  }

  return nd_->getQualifiedNameAsString();
}

std::string getMangledName(
  clang::MangleContext* mangleContext_,
  const clang::QualType& qt_,
  const model::FileLoc& fileLoc_)
{
  const clang::Type* type = getStrippedType(qt_);

  // TODO: How can this happen?
  if (!type)
    return std::string();

  // TODO: What about primitive types?
  if (const clang::TypedefType* td = type->getAs<clang::TypedefType>())
  {
    if (const clang::TypedefNameDecl* tDecl = td->getDecl())
      return getMangledName(mangleContext_, tDecl, fileLoc_);
  }
  else if (const clang::CXXRecordDecl* rDecl = type->getAsCXXRecordDecl())
    return getMangledName(mangleContext_, rDecl);
  else if (const clang::EnumType* et = type->getAs<clang::EnumType>())
    if (const clang::Decl* decl = et->getDecl())
      return getMangledName(
        mangleContext_,
        llvm::dyn_cast<clang::NamedDecl>(decl));

  return std::string();
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

}
}
