/*
 * parseutil.cpp
 *
 *  Created on: May 7, 2014
 *      Author: ezoltbo
 */

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include "parseutil.h"

namespace cc
{
namespace parser
{


bool isFunctionPointer(clang::DeclRefExpr* de)
{
  return isFunctionPointer(de->getType().getTypePtrOrNull());
}


bool isFunctionPointer(clang::ValueDecl* qt)
{
  return isFunctionPointer(qt->getType().getTypePtrOrNull());
}

bool isFunctionPointer(const clang::Type* type)
{
  while (type)
  {
    if (type->isFunctionType() ||
        type->isFunctionPointerType() ||
        type->isMemberFunctionPointerType())
    {
      return true;
    }
    else if (auto parenType = type->getAs<clang::ParenType>())
    {
      type = parenType->getInnerType().getTypePtrOrNull();
    }
    else if (auto arrayType = llvm::dyn_cast<clang::ArrayType>(type))
    {
      type = arrayType->getElementType().getTypePtrOrNull();
    }
    else if (type->isPointerType() || type->isReferenceType())
    {
      type = type->getPointeeType().getTypePtrOrNull();
    }
    else if (auto elaboratedType = llvm::dyn_cast<clang::ElaboratedType>(type))
    {
      type = elaboratedType->desugar().getTypePtrOrNull();
    }
    else
    {
      return false;
    }
  }

  return false;
}

const clang::Type* getStrippedType(clang::QualType qt)
{
  auto type = qt.getTypePtrOrNull();

  // strip ParenType, Pointer, Reference and ArrayType
  while (type)
  {
    if (type->getAs<clang::TypedefType>())
    {
      break;
    }

    if (auto parenType = type->getAs<clang::ParenType>())
    {
      type = parenType->getInnerType().getTypePtrOrNull();
    }
    else if (type->isReferenceType() || type->isPointerType())
    {
      type = type->getPointeeType().getTypePtrOrNull();
    }
    else if (auto arrayType = llvm::dyn_cast<clang::ArrayType>(type))
    {
      type = arrayType->getElementType().getTypePtrOrNull();
    }
    else
    {
      break;
    }
  }

  return type;
}

clang::Expr* getInnerExpr(clang::Stmt* expr)
{
  if (!llvm::isa<clang::Expr>(expr))
    return {};

  auto ret = static_cast<clang::Expr*>(expr);

  for(;;)
  {
    if (!ret)
    {
      return {};
    }


    if (llvm::isa<clang::CastExpr>(ret))
    {
      auto castExpr = static_cast<clang::CastExpr*>(ret);

      ret = castExpr->getSubExpr();
    }
    else if (llvm::isa<clang::ParenExpr>(ret))
    {
      auto parenExpr = static_cast<clang::ParenExpr*>(ret);

      ret = parenExpr->getSubExpr();
    }
    else
    {
      break;
    }
  }

  return ret;
}

clang::ValueDecl* getCalleeDecl(clang::CallExpr* ce)
{
  if (auto fd = ce->getDirectCallee())
  {
    if(!fd->isImplicit() &&
      !llvm::isa<clang::CXXConstructorDecl>(fd) &&
      !llvm::isa<clang::CXXDestructorDecl>(fd) )
    {
      return fd;
    }
    else
      return {};
  }

  if (auto cd = ce->getCalleeDecl())
  {
    if(llvm::isa<clang::DeclaratorDecl>(cd))
    {
      return static_cast<clang::DeclaratorDecl*>(cd);
    }
  }

  for (auto it = ce->child_begin(); it != ce->child_end(); ++it)
  {
    auto expr = getInnerExpr(*it);

    if (expr && llvm::isa<clang::ArraySubscriptExpr>(expr))
    {
      auto arrayExpr = static_cast<clang::ArraySubscriptExpr*>(expr);

      auto base = getInnerExpr(arrayExpr->getBase());
      if (base && llvm::isa<clang::DeclRefExpr>(base))
      {
        auto declRef = static_cast<clang::DeclRefExpr*>(base);
        auto decl = declRef->getDecl();

        return decl;
      }
    }
  }

  return {};
}


} // parse
} // cc



