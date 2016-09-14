#include <iostream>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/lazy-ptr.hxx>

#include "model/file.h"
#include "model/file-odb.hxx"

#include "symbolhelper.h"

namespace
{

std::string getNamespaceName(const clang::DeclContext* ctx)
{
  std::string nsName;
  if (const clang::DeclContext* declCtx = ctx->getParent())
  {
    if (const clang::NamedDecl* nDecl =
        llvm::dyn_cast<clang::NamedDecl>(declCtx))
    {
      nsName = nDecl->getQualifiedNameAsString();
    }
  }

  return nsName;
}

}

namespace cc
{
namespace parser
{

std::string SymbolHelper::getMangledName(clang::GlobalDecl gd_) const
{
  const clang::NamedDecl *nd = llvm::cast<clang::NamedDecl>(gd_.getDecl());

  //Workaround if no name
  if( nd->getDeclName().getAsString().size() == 0)
  {
    return "entity without name";
  }

  //No need to mangle for C sources or declaration in  extern "C"
  if (!_mgctx->shouldMangleDeclName(nd))
    return nd->getDeclName().getAsString();


  llvm::SmallString<256> Buffer;
  llvm::raw_svector_ostream Out(Buffer);
  if (const clang::CXXConstructorDecl *D = llvm::dyn_cast<clang::CXXConstructorDecl>(nd))
  {
    _mgctx->mangleCXXCtor(D, gd_.getCtorType(), Out);
  }
  else if (const clang::CXXDestructorDecl *D = llvm::dyn_cast<clang::CXXDestructorDecl>(nd))
  {
    _mgctx->mangleCXXDtor(D, gd_.getDtorType(), Out);
  }
  else if (const clang::BlockDecl *BD = llvm::dyn_cast<clang::BlockDecl>(nd))
  {
    _mgctx->mangleBlock(BD->getDeclContext(), BD, Out);
  }
  else
  {
    // FIXME: workaround for a crash reported as #236
    // From Clang's code:
    // We should never be mangling something without a prototype.
    const clang::FunctionDecl* FD = llvm::dyn_cast<clang::FunctionDecl>(nd);
    if (FD && !llvm::isa<clang::FunctionProtoType>(
                FD->getType()->getAs<clang::FunctionType>()))
    {
      return "Something without a prototype";
    }

    _mgctx->mangleName(nd, Out);
  }

  // Allocate space for the mangled name.
  Out.flush();
  std::string str(Buffer.begin(), Buffer.end());

  return str;
}

std::string SymbolHelper::getMangledNameOfLocal(clang::GlobalDecl gd_, model::CppAstNodePtr node) const
{
  return getMangledName(gd_) + getSuffixFromNode(node);
}

std::string SymbolHelper::getMangledNameOfField(clang::NamedDecl* nd_, model::CppAstNodePtr node) const
{
  return nd_->getQualifiedNameAsString();
}

std::string SymbolHelper::getMangledNameOfType(
  clang::RecordDecl* rd_,
  model::CppAstNodePtr node) const
{
  return getTypeQualifiedName(rd_);
}

std::string SymbolHelper::getMangledNameOfEnum(clang::EnumDecl* ed_, model::CppAstNodePtr node) const
{
  auto ret = ed_->getQualifiedNameAsString();

  if (ret.find("<anonymous>") != std::string::npos)
    ret += getSuffixFromNode(node);

  return  ret /* + getSuffixFromNode(node) */;
}

std::string SymbolHelper::getMangledNameOfTypedef(clang::TypedefNameDecl* td_, model::CppAstNodePtr node) const
{
  return td_->getQualifiedNameAsString() + getSuffixFromNode(node);
}

std::string SymbolHelper::getMangledNameOfEnumConstant(clang::EnumConstantDecl* ecd_, model::CppAstNodePtr node) const
{
  return ecd_->getQualifiedNameAsString() /* + getSuffixFromNode(node) */;
}

std::string SymbolHelper::getSuffixFromNode(const model::CppAstNodePtr& node) const
{
  if (node && node->location.file)
  {
    std::string defPos;
    defPos += std::to_string(node->location.file.object_id()) + ':';
    defPos += std::to_string(node->location.range.start.line) + ':';
    defPos += std::to_string(node->location.range.start.column);

    return defPos;
  }

  return std::to_string(reinterpret_cast<long int>(node.get()));
}


std::string SymbolHelper::getTypeName(const clang::RecordDecl* rd_) const
{
  const clang::NamedDecl* namedDecl = rd_;
  const clang::Type* type = rd_->getTypeForDecl();
 
  if (auto typedefDecl = rd_->getTypedefNameForAnonDecl())
  {
    if (const clang::Type* ttype = typedefDecl->getTypeForDecl())
    {
      type = ttype;
    }
    namedDecl = typedefDecl;
  }

  std::string typeName;
  if (!type)
  {
    typeName = namedDecl->getNameAsString();
  }
  else
  {
    clang::PrintingPolicy printPol(rd_->getParentASTContext().getLangOpts());
    clang::QualType::getAsStringInternal(type, clang::Qualifiers(), typeName,
      printPol);

    if (type->getTypeClass() != clang::Type::InjectedClassName)
    {
      // HACK: This is a "normal" class, so it contains the namespace.
      std::function<std::string(const std::string&)> stripper =
        [&stripper](const std::string& str)
        {
          std::string ret = str;

          auto sep_pos = std::min(str.find("<"), str.find(","));
          auto colons_pos = str.rfind("::", sep_pos);

          if (colons_pos != std::string::npos)
          {
            ret = str.substr(colons_pos + 2);
            sep_pos -= str.size() - ret.size();
            //colons_pos -= str.size() - ret.size();
          }

          if(sep_pos != std::string::npos && sep_pos + 1 < ret.size())
            ret = ret.substr(0, sep_pos + 1) + stripper(ret.substr(sep_pos + 1));

          return ret;
        };

      typeName = stripper(typeName);
    }
  }

  if (typeName.empty())
  {
    return "<anonymous>";
  }

  return typeName;
}

std::string
SymbolHelper::getTypeQualifiedName(const clang::RecordDecl* rd_) const
{
  std::string typeName = getTypeName(rd_);
  std::string ns = getNamespaceName(rd_);
  if (!ns.empty())
  {
    typeName = ns + "::" + typeName;
  }

  return typeName;
}

} //parser
} //cc
