#include <cstring>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

#include <model/fileloc.h>
#include <model/fileloc-odb.hxx>

#include "symbolhelper.h"

namespace
{

std::string getSuffixFromLoc(const cc::model::FileLoc& fileLoc_)
{
  if (!fileLoc_.file)
    return std::string();

  return
      std::to_string(fileLoc_.file.object_id()) + ':'
    + std::to_string(fileLoc_.range.start.line) + ':'
    + std::to_string(fileLoc_.range.start.column);
}

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

std::string getMangledName(
  clang::MangleContext* mangleContext_,
  const clang::NamedDecl* nd_,
  const model::FileLoc& fileLoc_)
{
  if (const clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(nd_))
  {
    std::string result = nd_->getQualifiedNameAsString();

    if (const clang::DeclContext* dc = nd_->getParentFunctionOrMethod())
    {
      result += ':' + getMangledName(
        mangleContext_,
        llvm::dyn_cast<clang::FunctionDecl>(dc));

      if (const clang::ParmVarDecl* pvd =
          llvm::dyn_cast<clang::ParmVarDecl>(nd_))
        result += ':' + std::to_string(pvd->getFunctionScopeIndex());
    }

    if (vd->isLocalVarDecl() || llvm::isa<clang::ParmVarDecl>(nd_))
      result += ':' + getSuffixFromLoc(fileLoc_);

    return result;
  }
  else if (llvm::isa<clang::NamespaceDecl>(nd_))
  {
    const clang::NamespaceDecl* na_ = llvm::dyn_cast<clang::NamespaceDecl>(nd_);
    return na_->isAnonymousNamespace()
      ? "anonymous-ns:" + std::to_string(fileLoc_.file.object_id())
      : nd_->getQualifiedNameAsString();
  }
  else if (const clang::CXXRecordDecl* rd
    = llvm::dyn_cast<clang::CXXRecordDecl>(nd_))
  {
    // AST nodes of type CXXRecordDecl contain another CXXRecordDecl node. The
    // outer one is for the definition (and its position is the whole
    // definition) and the inner one is for a declaration (of which the position
    // is the "class" keyword and the class name.
    // The qualified name of the inner node is ClassName::ClassName, although we
    // should choose the same mangled name i.e. the qualified name of its
    // parent.

    const clang::CXXRecordDecl* parent
      = llvm::dyn_cast<clang::CXXRecordDecl>(rd->getParent());

    return parent && parent->getName() == rd->getName()
      ? parent->getQualifiedNameAsString()
      : rd->getQualifiedNameAsString();
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
    
    // TODO: In the case when a function has a template and a non-template
    // version then their mangled name is the same as long as the template is
    // not instantiated. Distinguising them by their locations is not good,
    // because if there is a function which has two versions in an #ifdef ...
    // #else ... #endif section then every usage will also have two versions.
    // So if a function has a template and a non-template version at the same
    // time then querying the definition of the non-template usage the template
    // version also returns.
    return out.str();
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

}
}
