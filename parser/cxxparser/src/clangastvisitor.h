// $Id$

#ifndef CXXPARSER_CLANGASTVISITOR_H
#define CXXPARSER_CLANGASTVISITOR_H

#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <unordered_set>
#include <functional>

#include <clang/AST/RecursiveASTVisitor.h>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include <clang/AST/Mangle.h>
#include <clang/AST/GlobalDecl.h>

#include <odb/session.hxx>

#include <parser/commondefs.h>
#include <parser/sourcemanager.h>

#include <model/workspace.h>
// #include <grocker/transaction.h>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/cxx/cpppointeranalysis.h>
#include <model/cxx/cpppointeranalysis-odb.hxx>
#include <model/fileloc.h>
#include <model/file.h>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include "cxxparser/cxxparser.h"

#include "cxxparsesession.h"
#include "filelocutil.h"
#include "symbolhelper.h"
#include "refcollector.h"
#include "opnames.h"
#include "parseutil.h"
#include "tracer.h"

namespace cc
{
namespace parser
{

class ClangASTVisitor : public clang::RecursiveASTVisitor<ClangASTVisitor>
{
public:
  ClangASTVisitor( std::shared_ptr<model::Workspace> w_,
                   SourceManager& srcMgr_,
                   CxxParseSession& session,
                   CxxAstPersister &astPersister
                 ) :
    _w(w_), _srcMgr(srcMgr_),
    _clangSrcMgr(session.astContext.getSourceManager()),
    _symbolHelper(session.astContext),
    _clang2our(session.clang2our),
    _newNodes(session.newNodes),
    _astPersister(astPersister),
    _session(session)
  {
  }

  ~ClangASTVisitor()
  {
    // drop ast nodes without mangled name
    // also drop already persisted AST nodes
    auto it = _clang2our.begin();
    while (it != _clang2our.end())
    {
      auto node = it->second;
      if (!node || node->mangledName.empty())
      {
        // Erase nodes without mangled name
        it = _clang2our.erase(it);
        continue;
      }

      if (node->astValue.empty())
      {
        node->astValue = "<anonymous>";
      }

      node->mangledNameHash = util::fnvHash(node->mangledName);
      ++it;
    }

    //auto persistedNodes = _astPersister.persistAstNodes(_clang2our);
    //for (const auto& node : persistedNodes)

    // FIXME: if it works, the _newNodes vector should be eliminated
    _astPersister.persistAstNodes(_clang2our);
    _newNodes.reserve(_clang2our.size());
    for (const auto& node : _clang2our)
    {
      // this AST node has just been added to the database
      _newNodes.insert(node.first);
    }
    
    util::OdbTransaction trans(*_w->getDb());
    trans([this]() {
      
      for (model::CppPointerAnalysis& pa : _pointerAnalysis)
      {
        _w->getDb()->persist(pa);
      }
    });
  }

  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }

  bool TraverseTypeLoc( clang::TypeLoc TL_ )
  {
    clang::QualType T = TL_.getType();
    const clang::Type *type = T.getTypePtrOrNull();  //operator* of T is overloaded to return a Type&
    if(!type) return true;

    auto astnode = addAstNode(TL_.getOpaqueData(), model::CppAstNode::AstType::TypeLocation,
                              TL_.getBeginLoc(), TL_.getEndLoc());

    if (auto tdType = type->getAs<clang::TypedefType>())
    {
      if (auto tdDecl = tdType->getDecl())
      {
        astnode->astValue = tdDecl->getName().str();
        astnode->mangledName = _symbolHelper.getMangledNameOfTypedef(tdDecl,
          _clang2our[tdDecl]);
        astnode->symbolType = model::CppAstNode::SymbolType::Typedef;
      }
    }
    else if (auto record = type->getAsCXXRecordDecl())
    {
      astnode->astValue = record->getName().str();
      astnode->mangledName = _symbolHelper.getMangledNameOfType(record,
        _clang2our[record]);
      astnode->symbolType = model::CppAstNode::SymbolType::Type;
    }
    else if (auto enumType = type->getAs<clang::EnumType>())
    {
      if (auto enumDecl = enumType->getDecl())
      {
        astnode->astValue = enumDecl->getName().str();
        astnode->mangledName = _symbolHelper.getMangledNameOfEnum(enumDecl,
          _clang2our[enumDecl]);
        astnode->symbolType = model::CppAstNode::SymbolType::Enum;
      }
    }

    astnode->astType = tlState.getTypeLocType();

    _aststack.push_back(astnode);
    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseTypeLoc(TL_);
    _aststack.pop_back();

    return b;
  }

  bool TraverseDecl( clang::Decl *d_ )
  {
    if (!d_)
    {
      return clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseDecl(d_);
    }

    bool prevForceInvalid = _forceInvalid;
    _forceInvalid = d_->isImplicit();

    if(!_forceInvalid && d_->getDeclContext())
    {
      auto parentFunc = llvm::dyn_cast<clang::FunctionDecl>(d_->getDeclContext());
      if(parentFunc && parentFunc->isImplicit()) { _forceInvalid = true; }
    }

    auto astnode = addAstNode(d_, model::CppAstNode::AstType::Declaration,
                              d_->getLocStart(), d_->getLocEnd(), _forceInvalid);

    _aststack.push_back(astnode);
    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseDecl(d_);
    _aststack.pop_back();
    _forceInvalid = prevForceInvalid;
    return b;
  }

  bool TraverseStmt( clang::Stmt *s_ )
  {
    if(!s_) return true;

    auto astnode = addAstNode(s_, model::CppAstNode::AstType::Statement,
                              s_->getLocStart(), s_->getLocEnd());

    _aststack.push_back(astnode);
    bool writtenVars = collectWrittenVariablesIfNeed(s_);
    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseStmt(s_);
    if(writtenVars)
      _writtenNodes.pop_back();
    _aststack.pop_back();

    return b;
  }

  bool TraverseParmVarDecl( clang::ParmVarDecl *d_ )
  {
    tlState.inParmVar = true;

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseParmVarDecl(d_);

    tlState.inParmVar = false;

    return b;
  }

  bool TraverseVarDecl( clang::VarDecl *d_ )
  {
    tlState.inVar = true;

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseVarDecl(d_);

    tlState.inVar = false;

    return b;
  }

  bool TraverseFieldDecl( clang::FieldDecl *d_ )
  {
    tlState.inField = true;

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseFieldDecl(d_);

    tlState.inField = false;

    return b;
  }

  bool TraverseFunctionProtoTypeLoc( clang::FunctionProtoTypeLoc d_ )
  {
    tlState.inFunctionProto = true;

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseFunctionProtoTypeLoc(d_);

    tlState.inFunctionProto = false;

    return b;
  }

  bool TraverseCompoundStmt( clang::CompoundStmt *s )
  {
    bool didISwitchitOn = false;

    if (!tlState.inCompoundStmt)
    {
      tlState.inCompoundStmt = true;
      didISwitchitOn = true;
    }

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseCompoundStmt(s);

    if (didISwitchitOn)
      tlState.inCompoundStmt = false;

    return b;
  }

  bool VisitRecordDecl(clang::RecordDecl *rd)
  {
    bool isDef = rd->isCompleteDefinition();

    auto _last = getLastAstNode();
    if (!_last)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << rd->getQualifiedNameAsString();
      return true;
    }

    _last->astType = isDef ? model::CppAstNode::AstType::Definition :
                             model::CppAstNode::AstType::Declaration;
    _last->symbolType = model::CppAstNode::SymbolType::Type;
    _last->mangledName = _symbolHelper.getMangledNameOfType(rd, _last);

    /*if (auto typedefDecl = rd->getTypedefNameForAnonDecl())
    {
      _last->astValue = typedefDecl->getNameAsString();
    }*/
    _last->astValue = _symbolHelper.getTypeName(rd);

    for( auto it = rd->field_begin(); it != rd->field_end(); ++it)
    {
      if (_clang2our.find(*it) == _clang2our.end())
      {
        auto aNode =  addAstNode(*it, model::CppAstNode::AstType::Definition,
            (*it)->getLocStart(), (*it)->getLocEnd());

        aNode->astValue = (*it)->getNameAsString();
        aNode->mangledName = _symbolHelper.getMangledNameOfField(*it, aNode);
        aNode->symbolType = varOrFuncPtr(*it);
      }
    }

    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl *ed)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << ed->getQualifiedNameAsString();
      return true;
    }

    lastNd->astType = model::CppAstNode::AstType::Definition;
    lastNd->symbolType = model::CppAstNode::SymbolType::Enum;
    lastNd->mangledName = _symbolHelper.getMangledNameOfEnum(ed, lastNd);

    return true;
  }

  bool VisitTypedefNameDecl( clang::TypedefNameDecl *td)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << td->getQualifiedNameAsString();
      return true;
    }

    lastNd->astType = model::CppAstNode::AstType::Definition;
    lastNd->symbolType = model::CppAstNode::SymbolType::Typedef;
    lastNd->mangledName = _symbolHelper.getMangledNameOfTypedef(td, lastNd);

    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl *ec)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << ec->getQualifiedNameAsString();
      return true;
    }

    lastNd->astType = model::CppAstNode::AstType::Definition;
    lastNd->symbolType = model::CppAstNode::SymbolType::EnumConstant;
    lastNd->mangledName =
      _symbolHelper.getMangledNameOfEnumConstant(ec, lastNd);

    return true;
  }

  //TODO
  //bool TraverseClassTemplateDecl( clang::ClassTemplateDecl* ctd_)
//----------------- Visit Declarations -------------------------------

  //TODO: Destructor mangling too
  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *cd)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << cd->getQualifiedNameAsString();
      return true;
    }

    auto globalDecl = clang::GlobalDecl(cd, clang::CXXCtorType::Ctor_Complete);

    bool def = cd->isThisDeclarationADefinition();

    lastNd->astType = def ? model::CppAstNode::AstType::Definition :
                            model::CppAstNode::AstType::Declaration;
    lastNd->mangledName = _symbolHelper.getMangledName(globalDecl);;
    lastNd->symbolType = model::CppAstNode::SymbolType::Function;

    for (auto it = cd->init_begin(), e = cd->init_end(); it != e; ++it)
    {
      const clang::CXXCtorInitializer *ci = *it;
      if (!ci->getMember() || ci->getSourceOrder() == -1)
        continue;

      auto memberRefNode = addAstNode(ci, model::CppAstNode::AstType::Write,
        ci->getSourceRange().getBegin(), ci->getSourceRange().getEnd());

      auto member = ci->getMember();
      auto memberDeclNode = _clang2our[member];

      memberRefNode->astValue = ci->getMember()->getNameAsString();
      memberRefNode->mangledName =
        _symbolHelper.getMangledNameOfField(member, memberDeclNode);
      memberRefNode->symbolType = varOrFuncPtr(ci->getMember());
    }

    return true;
  }

  bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *dd)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << dd->getQualifiedNameAsString();
      return true;
    }

    auto gDecl = clang::GlobalDecl(dd, clang::CXXDtorType::Dtor_Complete);
    bool def = dd->isThisDeclarationADefinition();

    lastNd->astType = def ? model::CppAstNode::AstType::Definition :
                            model::CppAstNode::AstType::Declaration;
    lastNd->mangledName = _symbolHelper.getMangledName(gDecl);;
    lastNd->symbolType = model::CppAstNode::SymbolType::Function;

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* decl)
  {
    if(!llvm::isa<clang::CXXConstructorDecl>(decl) &&
        !llvm::isa<clang::CXXDestructorDecl>(decl))
    {
      auto lastNd = getLastAstNode();
      if (!lastNd)
      {
        SLog(util::ERROR)
          << "Empty AST Stack!! Could not add declaration of "
          << decl->getQualifiedNameAsString();
        return true;
      }

      auto gDecl = clang::GlobalDecl(decl);
      std::string mangledName = _symbolHelper.getMangledName(gDecl);

      if (decl->isMain())
      {
        mangledName += _symbolHelper.getSuffixFromNode(lastNd);
      }

      bool def = decl->isThisDeclarationADefinition();

      lastNd->astType = def ? model::CppAstNode::AstType::Definition :
                              model::CppAstNode::AstType::Declaration;
      lastNd->mangledName = std::move(mangledName);
      lastNd->symbolType = model::CppAstNode::SymbolType::Function;
    }

    return true;
  }
  

  bool VisitFieldDecl(clang::FieldDecl* fd)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << fd->getQualifiedNameAsString();
      return true;
    }

    std::string mangledName = _symbolHelper.getMangledNameOfField(fd, lastNd);

    lastNd->astType = model::CppAstNode::AstType::Definition;
    lastNd->mangledName = std::move(mangledName);
    lastNd->symbolType = varOrFuncPtr(fd);
    
    return true;
  }

  bool VisitVarDecl( clang::VarDecl* decl )
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << decl->getQualifiedNameAsString();
      return true;
    }

    lastNd->symbolType = varOrFuncPtr(decl);

    if (!decl->isLocalVarDecl() && !llvm::isa<clang::ParmVarDecl>(decl))
    {
      bool def = decl->isThisDeclarationADefinition() == clang::VarDecl::Definition;

      lastNd->astType = def ? model::CppAstNode::AstType::Definition :
                              model::CppAstNode::AstType::Declaration;
      lastNd->mangledName =
        _symbolHelper.getMangledName(clang::GlobalDecl(decl));
    }
    else
    {
      lastNd->astType = model::CppAstNode::AstType::Definition;
      lastNd->mangledName =
        _symbolHelper.getMangledNameOfLocal(clang::GlobalDecl(decl),
        lastNd);
    }

    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl* d)
  {
    auto lastNd = getLastAstNode();
    if (!lastNd)
    {
      SLog(util::ERROR)
        << "Empty AST Stack!! Could not add declaration of "
        << d->getQualifiedNameAsString();
      return true;
    }

    auto ctx = d->getDeclContext();
    auto funDecl = llvm::dyn_cast<clang::FunctionDecl>(ctx);
    
    std::shared_ptr<model::CppAstNode> funNode;
    
    if (funDecl)
    {
      if(_clang2our.find(funDecl) == _clang2our.end())
      {        
        funNode = std::make_shared<model::CppAstNode>();
        getFileLoc(funNode->location, funDecl->getLocStart(), funDecl->getLocEnd());
      }
      else
        funNode = std::make_shared<model::CppAstNode>(*_clang2our[funDecl]);
      
      for (unsigned int i = 0; i < funDecl->getNumParams(); ++i)
      {
        if (d == funDecl->getParamDecl(i))
        {          
          lastNd->mangledName = funNode->mangledName + "_" + 
                                _symbolHelper.getSuffixFromNode(funNode) + "_" +
                                std::to_string(i);
          bool def = funDecl->isThisDeclarationADefinition();
          lastNd->astType = def ? model::CppAstNode::AstType::Definition :
                                  model::CppAstNode::AstType::Declaration;
        }
      }
    }

    return true;
  }

//----------------- Visit Statements ---------------------------------
  
  bool VisitCXXConstructExpr(clang::CXXConstructExpr* ce_)
  {
    bool isVisible = false;
    const clang::CXXConstructorDecl* ctor = getCtorDecl(ce_, isVisible);
    if (!ctor)
    {
      return true;
    }

    auto lastNd = getLastAstNode();
    lastNd->mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(ctor,
      clang::CXXCtorType::Ctor_Complete)); // + "_" +  _symbolHelper.getSuffixFromNode(lastNd);
    lastNd->symbolType = model::CppAstNode::SymbolType::Function;
    lastNd->astType = model::CppAstNode::AstType::Usage;
    lastNd->astValue = "construct call " + ctor->getNameAsString();
    lastNd->visibleInSourceCode = isVisible;

    return true;
  }

  bool VisitCXXNewExpr(clang::CXXNewExpr* ce_)
  {
    auto funDecl = ce_->getOperatorNew();

    if (!funDecl)
      return true;

    if (llvm::isa<clang::FunctionDecl>(funDecl))
    {
      auto fd = static_cast<clang::FunctionDecl*>(funDecl);

      std::string call = fd->isTemplateInstantiation() ?
        "template call " : "call ";

      auto lastNd = getLastAstNode();
      auto gDecl = clang::GlobalDecl(fd);

      lastNd->mangledName = _symbolHelper.getMangledName(gDecl);      
      lastNd->symbolType = model::CppAstNode::SymbolType::Function;
      lastNd->astType = model::CppAstNode::AstType::Usage;
      lastNd->astValue = call + fd->getNameAsString();
    }
    else if(llvm::isa<clang::DeclaratorDecl>(funDecl))
    {
      // this is a call through a function pointer
      auto vd = static_cast<clang::DeclaratorDecl*>(funDecl);

      auto lastNd = getLastAstNode();

      if (_clang2our.find(vd) != _clang2our.end())
      {
        auto declNode = _clang2our[vd];

        lastNd->mangledName = declNode->mangledName;
        lastNd->symbolType = model::CppAstNode::SymbolType::FunctionPtr;
        lastNd->astType = model::CppAstNode::AstType::Usage;
        lastNd->astValue = "fptr call " + declNode->astValue;
      }
    }

    return true;
  }
  
  bool VisitCXXDestructor(clang::CXXDeleteExpr* de_)
  {
    const clang::Type *type = de_->getDestroyedType().getTypePtrOrNull();
    
    auto astNode = addAstNode(type, model::CppAstNode::AstType::Usage,
        de_->getLocStart(), de_->getLocEnd());
    if(auto rd = type->getAsCXXRecordDecl())
    {
      astNode->symbolType = model::CppAstNode::SymbolType::Function;
      astNode->astValue = "destruct call ~" + rd->getName().str();      
      
      for( auto it = rd->decls_begin(); it != rd->decls_end(); ++it)
      {      
        if (llvm::isa<clang::CXXDestructorDecl>(*it))
        {
          auto methodDecl = llvm::dyn_cast<clang::CXXDestructorDecl>(*it);
          if (_clang2our.find(methodDecl) != _clang2our.end())
          {            
            astNode->mangledName = _clang2our[methodDecl]->mangledName;
            astNode->location.range.start.column = -1;
            astNode->location.range.end.column = -1;
            astNode->visibleInSourceCode = false;
          }
        }   
      }
    }  
    return true;
  }
  
  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* de_)
  {
    auto lastNd = getLastAstNode();
    auto funDecl = de_->getOperatorDelete();

    if (!funDecl)
      return true;

    if (llvm::isa<clang::FunctionDecl>(funDecl))
    {
      auto fd = static_cast<clang::FunctionDecl*>(funDecl);

      std::string call = fd->isTemplateInstantiation() ?
        "template call " : "call ";

      auto lastNd = getLastAstNode();
      auto gDecl = clang::GlobalDecl(fd);

      lastNd->mangledName = _symbolHelper.getMangledName(gDecl);
      lastNd->symbolType = model::CppAstNode::SymbolType::Function;
      lastNd->astType = model::CppAstNode::AstType::Usage;
      lastNd->astValue = call + fd->getNameAsString();
      VisitCXXDestructor(de_);
    }
    else if(llvm::isa<clang::DeclaratorDecl>(funDecl))
    {
      // this is a call through a function pointer
      auto vd = static_cast<clang::DeclaratorDecl*>(funDecl);

      auto lastNd = getLastAstNode();

      if (_clang2our.find(vd) != _clang2our.end())
      {
        auto declNode = _clang2our[vd];

        lastNd->mangledName = declNode->mangledName;
        lastNd->symbolType = model::CppAstNode::SymbolType::FunctionPtr;
        lastNd->astType = model::CppAstNode::AstType::Usage;
        lastNd->astValue = "fptr call " + declNode->astValue;
        VisitCXXDestructor(de_);
      }
    }

    return true;
  }
  /**
   *  Visit function for new operator. Almost equal to VisitCallExpr.
   */
  bool VisitCallExpr(clang::CallExpr* ce_)
  {
    auto funDecl = getCalleeDecl(ce_);

    if (!funDecl)
      return true;

    if (llvm::isa<clang::FunctionDecl>(funDecl))
    {
      auto fd = static_cast<clang::FunctionDecl*>(funDecl);

      std::string call = fd->isTemplateInstantiation() ?
        "template call " : "call ";

      auto lastNd = getLastAstNode();
      auto gDecl = clang::GlobalDecl(fd);

      lastNd->mangledName = _symbolHelper.getMangledName(gDecl);
      lastNd->symbolType = model::CppAstNode::SymbolType::Function;
      lastNd->astType = isVirtualCall(ce_) ? model::CppAstNode::AstType::VirtualCall :
                                             model::CppAstNode::AstType::Usage;
      lastNd->astValue = call + fd->getNameAsString();
      
//      if(fd->isTemplateInstantiation())
//      {
//        lastNd->mangledName += "_" + _symbolHelper.getSuffixFromNode(lastNd);
//      }
    }
    else if(llvm::isa<clang::DeclaratorDecl>(funDecl))
    {
      // this is a call through a function pointer
      auto vd = static_cast<clang::DeclaratorDecl*>(funDecl);

      auto lastNd = getLastAstNode();

      if (_clang2our.find(vd) != _clang2our.end())
      {
        auto declNode = _clang2our[vd];

        lastNd->mangledName = declNode->mangledName;
        lastNd->symbolType = model::CppAstNode::SymbolType::FunctionPtr;
        lastNd->astType = model::CppAstNode::AstType::Usage;
        lastNd->astValue = "fptr call " + declNode->astValue;
      }
    }
    
    //visitPointerAnalyisCallExpr(ce_);
    
    return true;
  }

  bool isVirtualCall(clang::CallExpr* ce_)
  {
    if (auto mc = llvm::dyn_cast<clang::CXXMemberCallExpr>(ce_))
    {
      auto methDecl = mc->getMethodDecl();

      if (!methDecl || !methDecl->isVirtual())
      {
        return false;
      }      
      
      if (auto obj = mc->getImplicitObjectArgument())
      {        
        // Type is a pointer
        if (auto type = obj->getType().getTypePtrOrNull())
          if (type->isPointerType())
            return true;  
        
        // VarDecl reference type check
        if(auto declRef = llvm::dyn_cast<clang::DeclRefExpr>(obj))
          if(auto decl = declRef->getDecl())
            if(auto vDecl = llvm::dyn_cast<clang::VarDecl>(decl))
              if(auto vDeclType = vDecl->getType().getTypePtr())
                if(vDeclType->isReferenceType())
                  return true;        
      }
    }
    
    return false;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* d_)
  {
    clang::ValueDecl* decl = d_->getDecl();

    if(decl == 0) //TODO: mi legyen akkor ha a decl 0
    {
      return true;
    }

    auto astNode = _clang2our[decl];
    if (!astNode)
    {
      astNode = addAstNode(decl, model::CppAstNode::AstType::Declaration,
        decl->getLocStart(), decl->getLocEnd(), decl->isImplicit());
    }

    if (llvm::isa<clang::VarDecl>(decl))
    {

      auto var = static_cast<clang::VarDecl*>(decl);

      std::string mn;

      if (_clang2our.find(var) != _clang2our.end())
      {
        mn = _clang2our[var]->mangledName;
      }

      if (isFunctionPointer(d_))
      {
        if (_clang2our.find(var) != _clang2our.end())
        {
          if (isThereEnclosingCallExprOfThis(mn))
            return true; // we already added a node for this
        }
      }

      _aststack.back()->symbolType = varOrFuncPtr(d_);
      _aststack.back()->astType = isWritten(d_) ?
        model::CppAstNode::AstType::Write :
        model::CppAstNode::AstType::Read;
      _aststack.back()->astValue = "ref " + decl->getNameAsString();
      _aststack.back()->mangledName = std::move(mn);
    }
    else if (decl->getKind() == clang::Decl::EnumConstant)
    {
      auto enumCDecl = static_cast<clang::EnumConstantDecl*>(decl);

      _aststack.back()->mangledName =
        _symbolHelper.getMangledNameOfEnumConstant(enumCDecl, _aststack.back());
      _aststack.back()->astType = model::CppAstNode::AstType::Usage;
      _aststack.back()->symbolType = model::CppAstNode::SymbolType::EnumConstant;
    }
    else if (llvm::isa<clang::FunctionDecl>(decl))
    {
      if (llvm::isa<clang::CXXConstructorDecl>(decl)
        || llvm::isa<clang::CXXDestructorDecl>(decl))
        return true;

      auto funcDecl = static_cast<clang::FunctionDecl*>(decl);

      auto mn = _symbolHelper.getMangledName(clang::GlobalDecl(funcDecl));

      if (isThereEnclosingCallExprOfThis(mn))
        return true;

      _aststack.back()->astValue = "ref " + decl->getNameAsString();
      _aststack.back()->mangledName = std::move(mn);
      _aststack.back()->astType = model::CppAstNode::AstType::Read;
      _aststack.back()->symbolType = model::CppAstNode::SymbolType::Function;
    }

    return true;
  }

  bool VisitNamedDecl(clang::NamedDecl *decl)
  {
    _aststack.back()->astValue = decl->getNameAsString();
    return true;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl *nd)
  {
    auto& node = _aststack.back();
    node->symbolType = model::CppAstNode::SymbolType::Namespace;
    node->astType = model::CppAstNode::AstType::Declaration;

    if (nd->isAnonymousNamespace())
    {
      node->mangledName = "anonymous-ns: " +
        node->location.file->path;
    }
    else
    {
      _aststack.back()->mangledName = nd->getQualifiedNameAsString();
    }

    return true;
  }

  bool VisitMemberExpr( clang::MemberExpr* expr)
  {
    clang::ValueDecl* vd = expr->getMemberDecl();
    auto& lastNd = _aststack.back();

    if (llvm::isa<clang::CXXMethodDecl>(vd))
    {
      lastNd->astValue = vd->getQualifiedNameAsString();
      return true;
    }

    if(vd)
    {
      auto mn = _symbolHelper.getMangledNameOfField(vd, lastNd);

      if (isThereEnclosingCallExprOfThis(mn))
        return true;

      lastNd->mangledName = std::move(mn);
      lastNd->symbolType = varOrFuncPtr(vd);
      lastNd->astType = isWritten(expr) ? model::CppAstNode::AstType::Write :
                                          model::CppAstNode::AstType::Read;
      lastNd->astValue = vd->getQualifiedNameAsString();
    }

    return true;
  }

  bool VisitStringLiteral( clang::StringLiteral* s_)
  {
    auto astNode = addAstNode(s_, model::CppAstNode::AstType::Declaration,
                              s_->getLocStart(), s_->getLocEnd());
    astNode->symbolType = model::CppAstNode::SymbolType::StringLiteral;
    astNode->astType = model::CppAstNode::AstType::Declaration;
    astNode->mangledName = _symbolHelper.getSuffixFromNode(astNode);
    astNode->astValue = "construct call str";
    _aststack.push_back(astNode);
        
    return true;
  }
  
private:

  bool _forceInvalid = false;

  template <typename T>
  model::CppAstNodePtr addAstNode(const T* clangPtr,
                                  model::CppAstNode::AstType astType,
                                  const clang::SourceLocation& start,
                                  const clang::SourceLocation& end,
                                  bool forceInvalidLocation = false)
  {
    // suppose as branch-free
    forceInvalidLocation = forceInvalidLocation || _forceInvalid;

    auto address = reinterpret_cast<const void*>(clangPtr);

    if (_clang2our.find(address) != _clang2our.end() &&
        _clang2our[address])
    {
      return _clang2our[address];
    }

    auto astNode = std::make_shared<model::CppAstNode>();

    astNode->astType = astType;
    astNode->visibleInSourceCode = !forceInvalidLocation;

    if(!forceInvalidLocation)
    {
      getFileLoc(astNode->location, start, end);
    }
    else
    {
      astNode->location.file = {};
      astNode->location.range = {};
    }

    _clang2our[address] = astNode;

    return astNode;
  }

  bool getFileLoc(
    model::FileLoc& astNodeLoc,
    const clang::SourceLocation& start,
    const clang::SourceLocation& end)
  {
    if(start.isValid() && end.isValid())
    {
      auto realStart = start;
      auto realEnd = end;

      if (_clangSrcMgr.isMacroArgExpansion(realStart))
        realStart = _clangSrcMgr.getSpellingLoc(start);

      if (_clangSrcMgr.isMacroArgExpansion(realEnd))
        realEnd = _clangSrcMgr.getSpellingLoc(end);

      FileLocUtil flu(_srcMgr, _clangSrcMgr);

      model::FileLoc fileLoc;
      if (flu.setInfo(realStart, realEnd, fileLoc))
      {
        astNodeLoc = fileLoc;
        return true;
      }
    }
    
    return false;
  }

  bool isWritten(const void* node)
  {
    for( const auto& s : _writtenNodes )
    {
      if( s.count(node) != 0 )
      {
        return true;
      }
    }
    return false;
  }

  model::CppAstNodePtr getLastAstNode()
  {
    if (!_aststack.empty())
      return _aststack.back();

    return {};
  }

  /**
    * Get function parameter managled name
    * @param decl - function declaraion
    * @param paramIndex - index of parameter
    * @return mangled name of parameter decl
    */
   uint64_t getParamVarMangledName(clang::ParmVarDecl * d, const int& paramIndex)
   {      
     auto ctx = d->getDeclContext();
     auto funDecl = llvm::dyn_cast<clang::FunctionDecl>(ctx);

     std::shared_ptr<model::CppAstNode> funcNode;

     if(_clang2our.find(funDecl) == _clang2our.end())
     {        
       funcNode = std::make_shared<model::CppAstNode>();
       getFileLoc(funcNode->location, funDecl->getLocStart(), funDecl->getLocEnd());
     }
     else
       funcNode = std::make_shared<model::CppAstNode>(*_clang2our[funDecl]);

     auto  mangledName =  funcNode->mangledName + "_" + 
            _symbolHelper.getSuffixFromNode(funcNode) + "_" +
            std::to_string(paramIndex)
     ; 
     
     if(mangledName != "")
       return util::fnvHash(mangledName); 
     return 0;     
   };
    
   /**
    * Get mangled name of a variable declaration
    * @param decl - variable declaration
    * @return mangled name of variable decl
    */
   uint64_t getVarMangledName(clang::VarDecl* decl)
   {
     std::string mangledName;
     if (llvm::isa<clang::ParmVarDecl>(decl)) // Function parameter decl
     {
       auto paramDecl = llvm::dyn_cast<clang::ParmVarDecl>(decl);
       if(auto ctx = decl->getDeclContext())
       {
         auto funDecl = llvm::dyn_cast<clang::FunctionDecl>(ctx);
         return getParamVarMangledName(paramDecl,paramDecl->getFunctionScopeIndex());
       }
     }
     else // VarDecl
     {
       if (!decl->isLocalVarDecl() && !llvm::isa<clang::ParmVarDecl>(decl))
       {
         mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(decl));
       }
       else
       {
         auto astNode = std::make_shared<model::CppAstNode>();
         getFileLoc(astNode->location, decl->getLocStart(), decl->getLocEnd());
         mangledName = _symbolHelper.getMangledNameOfLocal(clang::GlobalDecl(decl), astNode);
       }     
     }  
     if(mangledName != "")
       return util::fnvHash(mangledName);
     else 
       return 0;
   };
   
   /**
     * Visit the assign side expression and return the mangledNameHash of expr
     * @param expr - Expr
     * @param operators - operator kind
     * @return mangledNameHash of parameter expression
     */
  uint64_t visitAssignSideExpr(clang::Expr *expr, std::string& operators)
  {      
    if (llvm::isa<clang::UnaryOperator>(expr)) // Unary operator
    {
      auto unop = llvm::dyn_cast<clang::UnaryOperator>(expr);
      auto optCode = unop->getOpcode();
      if(optCode == clang::UnaryOperatorKind::UO_AddrOf)
        operators += "&";
      else if(optCode == clang::UnaryOperatorKind::UO_Deref)
        operators += "*";

      return visitAssignSideExpr(unop->getSubExpr(), operators);
    } 
    if(llvm::isa<clang::CXXNullPtrLiteralExpr>(expr))
    {
      return util::fnvHash("nullptr");
    }
    if (llvm::isa<clang::ImplicitCastExpr>(expr)) // Implicit cast expression
    {
      auto castExpr = llvm::dyn_cast<clang::ImplicitCastExpr>(expr);
      if(castExpr->isNullPointerConstant(_session.astContext, 
            clang::Expr::NullPointerConstantValueDependence::NPC_ValueDependentIsNotNull
          ) != clang::Expr::NullPointerConstantKind::NPCK_NotNull) // it's a nullpointer
        return util::fnvHash("nullptr");
      else
      return visitAssignSideExpr(castExpr->getSubExpr(), operators);                   
    }
    if(llvm::isa<clang::StringLiteral>(expr))
    {
      auto strExpr = llvm::dyn_cast<clang::StringLiteral>(expr);
      auto astNode = std::make_shared<model::CppAstNode>();
      getFileLoc(astNode->location, strExpr->getLocStart(), strExpr->getLocEnd());
      auto mangledName = _symbolHelper.getSuffixFromNode(astNode);      
      operators += "&"; // for pointer analysis algorithm
      
      return util::fnvHash(mangledName);
    }
    if (llvm::isa<clang::DeclRefExpr>(expr)) // Variable reference expression
      if(auto declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr))        
        if(auto valueDecl = llvm::dyn_cast<clang::ValueDecl>(declRef->getDecl()))
          if(auto varDecl = llvm::dyn_cast<clang::VarDecl>(valueDecl))
          {
            return getVarMangledName(varDecl);
          }
    if(llvm::isa<clang::CXXNewExpr>(expr)) // new expression
      if(auto newExpr = static_cast<clang::CXXNewExpr*>(expr))
        if(auto ctorExpr = newExpr->getConstructExpr())
        {
          bool isVisible = false;
          const clang::CXXConstructorDecl* ctor = getCtorDecl(ctorExpr, isVisible);
                    
          std::shared_ptr<model::CppAstNode> funNode;
          if(!_clang2our[ctorExpr])
          {
            funNode = std::make_shared<model::CppAstNode>();
            getFileLoc(funNode->location, ctorExpr->getLocStart(), ctorExpr->getLocEnd());
          
            auto mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(ctor,
              clang::CXXCtorType::Ctor_Complete)); // + "_" +  _symbolHelper.getSuffixFromNode(funNode);
            
            funNode->mangledName = mangledName;
            funNode->symbolType = model::CppAstNode::SymbolType::Function;
            funNode->astType = model::CppAstNode::AstType::Usage;
            funNode->astValue = "construct call " + ctor->getNameAsString();
            funNode->visibleInSourceCode = isVisible;
    
            operators += "&"; // it's for pointer analysis algorithm 

            if(mangledName != "")
            {
              auto id = _astPersister.createIdentifier(*funNode);
              return util::fnvHash(id);
            }
          }
        }
    if(llvm::isa<clang::MaterializeTemporaryExpr>(expr))
    {
      auto tExpr = static_cast<clang::MaterializeTemporaryExpr*>(expr);
      return visitAssignSideExpr(tExpr->GetTemporaryExpr(), operators);
    }
    if(llvm::isa<clang::CXXBindTemporaryExpr>(expr))
    {
      auto bindExpr = static_cast<clang::CXXBindTemporaryExpr*>(expr);
      return visitAssignSideExpr(bindExpr->getSubExpr(), operators);
    }
    if(llvm::isa<clang::CXXConstructExpr>(expr))
    {
      auto ctorExpr = static_cast<clang::CXXConstructExpr*>(expr);
      if(auto decl = ctorExpr->getConstructor())
      {
        for(unsigned i=0; i< ctorExpr->getNumArgs() && i < decl->getNumParams(); ++i)
        {
          auto argExpr = ctorExpr->getArg(i);
          return visitAssignSideExpr(argExpr, operators);
        }
      }
    }
    if(llvm::isa<clang::MemberExpr>(expr))
    {
      auto memberExpr = static_cast<clang::MemberExpr*>(expr);
      if(auto memberDecl = memberExpr->getMemberDecl())
        if(llvm::isa<clang::FieldDecl>(memberDecl))
        {
          auto fieldDecl = static_cast<clang::FieldDecl*>(memberDecl);
          auto astNode = std::make_shared<model::CppAstNode>();
          getFileLoc(astNode->location, fieldDecl->getLocStart(), fieldDecl->getLocEnd());
         
          auto mangledName = _symbolHelper.getMangledNameOfField(fieldDecl, astNode);
          if(mangledName != "")
            return util::fnvHash(mangledName);
        }
    }
    if(llvm::isa<clang::CXXMemberCallExpr>(expr))
    {
      auto memberCallExpr = static_cast<clang::CXXMemberCallExpr*>(expr);
      auto fd = memberCallExpr->getMethodDecl();
      
      if (!fd)
        return 0;
      
      std::string call = fd->isTemplateInstantiation() ?
        "template call " : "call ";

      std::shared_ptr<model::CppAstNode> funNode = std::make_shared<model::CppAstNode>();
      getFileLoc(funNode->location, memberCallExpr->getLocStart(), memberCallExpr->getLocEnd());

      auto mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(fd));
      
      funNode->mangledName = mangledName;
      funNode->symbolType = model::CppAstNode::SymbolType::Function;
      funNode->astType = isVirtualCall(memberCallExpr) ? model::CppAstNode::AstType::VirtualCall :
                                             model::CppAstNode::AstType::Usage;
      funNode->astValue = call + fd->getNameAsString();    

      operators += "&"; // it's for pointer analysis algorithm 
      if(mangledName != "")
      {
        auto id = _astPersister.createIdentifier(*funNode);
        return util::fnvHash(id);
      }
    }    
     
    return 0;
  };
    
  /**
    * Put an element into pointerAnalysis collection
    * @param pointerAnalysis - pointer analysis row
    */  
  void pushPointerAnalysis(model::CppPointerAnalysis pointerAnalysis)
  {
    if(pointerAnalysis.lhs != 0 && pointerAnalysis.rhs != 0 &&
      pointerAnalysis.lhs != pointerAnalysis.rhs /*&&
      (pointerAnalysis.lhsOperators != "" || pointerAnalysis.rhsOperators != "")*/)
      _pointerAnalysis.emplace_back(std::move(pointerAnalysis));
  };  
   
  void visitPointerAnalyisCallExpr(clang::CallExpr* call)
  {
    if(auto decl = call->getDirectCallee())
    { 
      
      /**
       * Get pointer analysis data
       * We need to store the call parameter index and declaration parameter index, 
       * because in lambda function there are an extra argument in argument range
      */
      int callParamIndex = call->getNumArgs() - 1;
      int declParamIndex = decl->getNumParams() - 1;
      while(callParamIndex >= 0 && declParamIndex >= 0)  
      {
        auto callParamExpr = call->getArg(callParamIndex);

        model::CppPointerAnalysis pointerAnalysis;

        if(decl) // parameter in declaration
        {
          std::string lhsOperators = "";
          auto paramDecl = decl->getParamDecl(declParamIndex);
          if(auto vDeclType = paramDecl->getType().getTypePtr())
            for(auto c : paramDecl->getType().getAsString())
              if(c == '&')
                lhsOperators += c;

          pointerAnalysis.lhs = getParamVarMangledName(paramDecl, declParamIndex);
          pointerAnalysis.lhsOperators = lhsOperators;            
        }

        // parameter in usage
        std::string rhsOperators = "";          

        pointerAnalysis.rhs = visitAssignSideExpr(callParamExpr, rhsOperators); 
        
        pointerAnalysis.rhsOperators = rhsOperators;
        
        pushPointerAnalysis(pointerAnalysis);      

        --callParamIndex;
        --declParamIndex;
      } 
    }
  }
  /**
   * Get pointer analysis data
   */
  void getPointerAnalysisData(clang::Stmt* s_)
  {    
    if (llvm::isa<clang::BinaryOperator>(s_))
    {
      clang::BinaryOperator* binop = static_cast<clang::BinaryOperator*>(s_);
            
      // Get pointer analysis
      model::CppPointerAnalysis pointerAnalysis;
      
      auto lhs = binop->getLHS();
      auto rhs = binop->getRHS();
      
      enum class AssignmentSide
      {
        Left = 0,
        Right
      };
      
      /**
       * Get pointer analysis data of an assignment side
       * @param _expr - one side of an assign expression 
       * @param side - side of an assign (left or right)
       * @return 
       */
      auto visitAssignmentSide = [&, this](clang::Expr * _expr, AssignmentSide side)
      {
        std::string operators = "";
        uint64_t mangledNameHash = 0;
        
        mangledNameHash = visitAssignSideExpr(_expr, operators);  
        
        if(side == AssignmentSide::Left)  // LEFT SIDE OF ASSIGN
        {
          pointerAnalysis.lhs = mangledNameHash;
          pointerAnalysis.lhsOperators = operators;
        }
        else // RIGHT SIDE OF ASSIGN
        { 
          pointerAnalysis.rhs = mangledNameHash;
          pointerAnalysis.rhsOperators = operators;
        }       
      };
      
      visitAssignmentSide(lhs, AssignmentSide::Left);
      visitAssignmentSide(rhs, AssignmentSide::Right);  
      
      pushPointerAnalysis(pointerAnalysis);      
    }
    else if (llvm::isa<clang::DeclStmt>(s_))
    {
      // Get pointer analysis data
      model::CppPointerAnalysis pointerAnalysis;
      auto declStmt = static_cast<clang::DeclStmt*>(s_);
      for(auto stmtDecl = declStmt->decl_begin(); stmtDecl != declStmt->decl_end(); ++stmtDecl)
      {
        if(llvm::isa<clang::VarDecl>(*stmtDecl))
        {
          auto varDecl = static_cast<clang::VarDecl*>(*stmtDecl);
          for(auto c : varDecl->getType().getAsString())
            if(c == '&')
              pointerAnalysis.lhsOperators += c;
          
          if(auto init = varDecl->getInit())
          {
            if(llvm::isa<clang::CallExpr>(init)) // if the right side is a call expr
            {
              auto callExpr = static_cast<clang::CallExpr*>(init);
              auto funNode = std::make_shared<model::CppAstNode>();
              getFileLoc(funNode->location, callExpr->getLocStart(), callExpr->getLocEnd());
              
              auto funDecl = getCalleeDecl(callExpr);
              if (funDecl && llvm::isa<clang::FunctionDecl>(funDecl))
              {
                auto fd = static_cast<clang::FunctionDecl*>(funDecl);
                auto mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(fd)); 
                
                std::string call = fd->isTemplateInstantiation() ?
                  "template call " : "call ";
                
                funNode->astValue = call + fd->getNameAsString();
                funNode->mangledName = mangledName;
                funNode->symbolType = model::CppAstNode::SymbolType::Function;
                funNode->astType = isVirtualCall(callExpr) ? model::CppAstNode::AstType::VirtualCall :
                                                                       model::CppAstNode::AstType::Usage;
              
                pointerAnalysis.rhs = util::fnvHash(_astPersister.createIdentifier(*funNode));
                pointerAnalysis.rhsOperators = "&";
                
                pointerAnalysis.lhs = getVarMangledName(varDecl);
              }
            }
            else if(llvm::isa<clang::ExprWithCleanups>(init)) // for smart pointers
            {
              auto exprCleanup = static_cast<clang::ExprWithCleanups*>(init);
              auto subExpr = exprCleanup->getSubExpr();
              if(llvm::isa<clang::CXXConstructExpr>(subExpr))
              {
                auto ctorExpr = static_cast<clang::CXXConstructExpr*>(subExpr);
                if(auto decl = ctorExpr->getConstructor())
                {
                  for(unsigned i=0; i< ctorExpr->getNumArgs() && i < decl->getNumParams(); ++i)
                  {
                    auto argExpr = ctorExpr->getArg(i);
                    if(llvm::isa<clang::MaterializeTemporaryExpr>(argExpr))
                    {
                      auto tExpr = static_cast<clang::MaterializeTemporaryExpr*>(argExpr);
                      auto tempExpr = tExpr->GetTemporaryExpr();
                      if(llvm::isa<clang::CXXBindTemporaryExpr>(tempExpr))
                      {
                        auto bindExpr = static_cast<clang::CXXBindTemporaryExpr*>(tempExpr);
                       
                        auto tempSubExpr = bindExpr->getSubExpr();
                        if(llvm::isa<clang::ImplicitCastExpr>(tempSubExpr))
                        {
                          auto castExpr = static_cast<clang::ImplicitCastExpr*>(tempSubExpr);
                          //pointerAnalysis.lhsOperators += "*";
                          pointerAnalysis.lhs = getVarMangledName(varDecl);

                          std::string rhsOperators = "";   
                          pointerAnalysis.rhs = visitAssignSideExpr(castExpr->getSubExpr(), rhsOperators); 
                          pointerAnalysis.rhsOperators = rhsOperators;                
                        }
                        else if(llvm::isa<clang::CallExpr>(tempSubExpr))
                        {
                          auto callExpr = static_cast<clang::CallExpr*>((bindExpr->getSubExpr()));
                          auto funDecl = getCalleeDecl(callExpr);

                          if (!funDecl)
                            return;

                          if (llvm::isa<clang::FunctionDecl>(funDecl))
                          {
                            auto fd = static_cast<clang::FunctionDecl*>(funDecl);
                            auto gDecl = clang::GlobalDecl(fd);
                            
                            
                            std::shared_ptr<model::CppAstNode> funNode;
                            if(!_clang2our[tExpr])
                            {
                              std::string mangledName = _symbolHelper.getMangledName(gDecl);
                              
                              funNode = std::make_shared<model::CppAstNode>();
                              getFileLoc(funNode->location, tExpr->getLocStart(), tExpr->getLocEnd());
                              
                              //mangledName += "_" + _symbolHelper.getSuffixFromNode(funNode);
                              
                              pointerAnalysis.lhs = getVarMangledName(varDecl);

                              funNode->mangledName = mangledName;
                              funNode->symbolType = model::CppAstNode::SymbolType::Function;
                              funNode->astType = isVirtualCall(callExpr) ? model::CppAstNode::AstType::VirtualCall :
                                                                     model::CppAstNode::AstType::Usage;
                              funNode->astValue = "template call " + fd->getNameAsString();
      
                              pointerAnalysis.rhs = util::fnvHash(_astPersister.createIdentifier(*funNode));
                              pointerAnalysis.rhsOperators = "&";
                            }                            
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            else if(llvm::isa<clang::CXXConstructExpr>(init))
            {
              auto ctorExpr = static_cast<clang::CXXConstructExpr*>(init);              
              if(auto decl = ctorExpr->getConstructor())
              {
                for(unsigned i=0; i< ctorExpr->getNumArgs() && i < decl->getNumParams(); ++i)
                {
                  auto argExpr = ctorExpr->getArg(i);
                  if(llvm::isa<clang::CXXNewExpr>(argExpr))
                  {
                    pointerAnalysis.lhs = getVarMangledName(varDecl);
                    
                    std::string rhsOperators = "";   
                    pointerAnalysis.rhs = visitAssignSideExpr(argExpr, rhsOperators);
                    pointerAnalysis.rhsOperators = rhsOperators;
                    
//                    auto newExpr = static_cast<clang::CXXNewExpr*>(argExpr);
//                    if(auto constructExpr = newExpr->getConstructExpr())
//                    {
//                      bool isVisible = false;
//                      const clang::CXXConstructorDecl* ctor = getCtorDecl(constructExpr, isVisible);
//
//                      if (!ctor)
//                        return;
//              
//                      if(!_clang2our[ctorExpr])
//                      {
//                        auto funNode = std::make_shared<model::CppAstNode>();
//                        getFileLoc(funNode->location, constructExpr->getLocStart(), constructExpr->getLocEnd());
//
//                        std::string mangledName = _symbolHelper.getMangledName(clang::GlobalDecl(ctor,
//                          clang::CXXCtorType::Ctor_Complete)) + "_" +  _symbolHelper.getSuffixFromNode(funNode);
//
//                        pointerAnalysis.lhs = getVarMangledName(varDecl);
//
//                        pointerAnalysis.rhs = util::fnvHash(mangledName);
//                        pointerAnalysis.rhsOperators = "&";
//
//                      }
//                    }
                  }
                  else if(llvm::isa<clang::ImplicitCastExpr>(argExpr))
                  {
                    auto castExpr = static_cast<clang::ImplicitCastExpr*>(argExpr);
                    
                    pointerAnalysis.lhs = getVarMangledName(varDecl);
                        
                    std::string rhsOperators = "";   
                    pointerAnalysis.rhs = visitAssignSideExpr(castExpr->getSubExpr(), rhsOperators); 
                    pointerAnalysis.rhsOperators = rhsOperators;
                  }
                  else if(llvm::isa<clang::CallExpr>(argExpr))
                  {
                    auto call = static_cast<clang::CallExpr*>((argExpr));
                    if(auto decl = call->getDirectCallee())
                    { 
                      if(call->getNumArgs() >= 1)
                      {
                        auto rhs = call->getArg(0);
                        pointerAnalysis.lhs = getVarMangledName(varDecl);

                        std::string rhsOp = "";
                        pointerAnalysis.rhs = visitAssignSideExpr(rhs, rhsOp);
                        pointerAnalysis.rhsOperators = rhsOp;

                        pushPointerAnalysis(pointerAnalysis);   
                      }       
                    }
                  }
                }
              }
            }
            else
            {               
              pointerAnalysis.lhs = getVarMangledName(varDecl);              
              std::string rhsOperators = "";           
              pointerAnalysis.rhs = visitAssignSideExpr(init, rhsOperators);
              pointerAnalysis.rhsOperators = rhsOperators;
            }
            
            pushPointerAnalysis(pointerAnalysis);      
          }
        }
        break;
      }
    }
    else if(llvm::isa<clang::CXXOperatorCallExpr>(s_))
    {
      model::CppPointerAnalysis pointerAnalysis;
      auto call = static_cast<clang::CXXOperatorCallExpr*>(s_);            
            
      if(auto decl = call->getDirectCallee())
      { 
        if(call->getOperator() == clang::OverloadedOperatorKind::OO_Equal)
        {
          if(call->getNumArgs() >= 2)
          {
            auto lhs = call->getArg(0);
            auto rhs = call->getArg(1);

            std::string lhsOp = "";
            pointerAnalysis.lhs = visitAssignSideExpr(lhs, lhsOp);
            pointerAnalysis.lhsOperators = lhsOp;

            std::string rhsOp = "";
            pointerAnalysis.rhs = visitAssignSideExpr(rhs, rhsOp);
            pointerAnalysis.rhsOperators = rhsOp;

            pushPointerAnalysis(pointerAnalysis);         
          }
        }else
        {
          for(size_t i = 0; i < decl->getNumParams() && i < call->getNumArgs(); ++i)
          {
            model::CppPointerAnalysis pointerAnalysis;

            auto lhs = decl->getParamDecl(i);
            auto rhs = call->getArg(i);            

            std::string lhsOp = "";
            pointerAnalysis.lhs = getParamVarMangledName(lhs, i);
            pointerAnalysis.lhsOperators = lhsOp + "*";

            std::string rhsOp = "";
            pointerAnalysis.rhs = visitAssignSideExpr(rhs, rhsOp);
            pointerAnalysis.rhsOperators = rhsOp;

            pushPointerAnalysis(pointerAnalysis); 
          }      
        }
        
      }
    }     
    else if(llvm::isa<clang::ExprWithCleanups>(s_))
    {
      auto exprCleanup = static_cast<clang::ExprWithCleanups*>(s_);
      auto subExpr = exprCleanup->getSubExpr();
      //getPointerAnalysisData(subExpr);
    }
  }
  bool collectWrittenVariablesIfNeed(clang::Stmt* s_)
  {
    auto isWritable = [](clang::QualType param)
    {
      auto typePtr = param.getTypePtr();
      return ((typePtr->isReferenceType() ||
          typePtr->isPointerType()) &&
        !typePtr->getPointeeType().isConstQualified()) ||
      typePtr->isRValueReferenceType();
    };

    std::vector<clang::Stmt*> collectFrom;

    while (llvm::isa<clang::CaseStmt>(s_))
    {
      auto caseStmt = static_cast<clang::CaseStmt*>(s_);

      if (caseStmt->getSubStmt())
      {
        s_ = caseStmt->getSubStmt();
      }
      else
      {
        break;
      }
    }
    
    //getPointerAnalysisData(s_);
      
    if (llvm::isa<clang::BinaryOperator>(s_))
    {
      clang::BinaryOperator* binop = static_cast<clang::BinaryOperator*>(s_);
      if( binop->isAssignmentOp() ||
        binop->isCompoundAssignmentOp() ||
        binop->isShiftAssignOp() )
      {
        collectFrom.push_back(binop->getLHS());
      }        
    }
    else if(llvm::isa<clang::UnaryOperator>(s_))
    {
      clang::UnaryOperator* unop = static_cast<clang::UnaryOperator*>(s_);
      if(unop->isIncrementDecrementOp())
      {
        collectFrom.push_back(unop->getSubExpr());
      }
    }
    else if (llvm::isa<clang::CXXOperatorCallExpr>(s_))
    {   
      auto call = static_cast<clang::CXXOperatorCallExpr*>(s_);
            
      if(auto decl = call->getDirectCallee())
      {        
        if (llvm::isa<clang::CXXMethodDecl>(decl))
        {
          auto method = static_cast<clang::CXXMethodDecl*>(decl);

          if (method && !method->isConst() && call->getNumArgs())
          {
            collectFrom.push_back(call->getArg(0));
          }

          for (int i=0; i < (int)decl->getNumParams() && i < (int)(call->getNumArgs()) - 1; ++i)
          {
            clang::QualType paramType = decl->getParamDecl(i)->getType();
            if (isWritable(paramType))
            {
              collectFrom.push_back(call->getArg(i+1));
            }
          }
        }
        else
        {
          for(unsigned i=0; i< call->getNumArgs() && i < decl->getNumParams(); ++i)
          {
            clang::QualType paramType = decl->getParamDecl(i)->getType();
            if (isWritable(paramType))
            {
              collectFrom.push_back(call->getArg(i));
            }
          }
        }
      }
    }
    else if (llvm::isa<clang::CallExpr>(s_))
    {
      clang::CallExpr* call = static_cast<clang::CallExpr*>(s_);

      if(auto decl = call->getDirectCallee())
      {
        for(unsigned i=0; i< call->getNumArgs() && i < decl->getNumParams(); ++i)
        {
          clang::QualType paramType = decl->getParamDecl(i)->getType();
          if (isWritable(paramType))
          {
            collectFrom.push_back(call->getArg(i));
          }
        }
      }
    }
    else if (llvm::isa<clang::CXXConstructExpr>(s_))
    {
      auto constructExpr = static_cast<clang::CXXConstructExpr*>(s_);

      if(auto decl = constructExpr->getConstructor())
      {
        for(unsigned i=0; i< constructExpr->getNumArgs() && i < decl->getNumParams(); ++i)
        {
          clang::QualType paramType = decl->getParamDecl(i)->getType();
          if (isWritable(paramType))
          {
            collectFrom.push_back(constructExpr->getArg(i));
          }
        }
      }
    }
    else if(llvm::isa<clang::MemberExpr>(s_))
    {
      auto memberExpr = static_cast<clang::MemberExpr*>(s_);
      auto memberDecl = memberExpr->getMemberDecl();

      if (memberDecl && llvm::isa<clang::CXXMethodDecl>(memberDecl))
      {
        auto cxxMethod = static_cast<clang::CXXMethodDecl*>(memberDecl);

        if (!cxxMethod->isConst())
        {
          collectFrom.push_back(memberExpr->getBase());
        }
      }
    }
    else if(llvm::isa<clang::CXXDeleteExpr>(s_))
    {
      collectFrom.push_back(s_);
    }

    if (!collectFrom.empty())
    {
      _writtenNodes.push_back(std::unordered_set<const void*>());
      for (auto stmt : collectFrom)
      {
        RefCollector refCollector(_writtenNodes.back());
        refCollector.collect(stmt);
      }
    }
    return !collectFrom.empty();
  }
  
  bool isThereEnclosingCallExprOfThis(const std::string mangledName)
  {
    for (int i = _aststack.size(); i > 0;)
    {
      --i;

      if ((_aststack[i]->symbolType == model::CppAstNode::SymbolType::Function ||
           _aststack[i]->symbolType == model::CppAstNode::SymbolType::FunctionPtr) &&
         _aststack[i]->astType == model::CppAstNode::AstType::Usage)
      {
        // this is f in call f()
        if (_aststack[i]->mangledName == mangledName)
        {
          return true;
        }
        // this could be f in call g(f) where g takes f as a parameter
        else
          return false;
      }
    }

    return false;
  };

  template <typename T>
  model::CppAstNode::SymbolType varOrFuncPtr(T* p)
  {
    if (isFunctionPointer(p))
      return model::CppAstNode::SymbolType::FunctionPtr;

    return model::CppAstNode::SymbolType::Variable;
  }

  /**
   * Returns the constructor decl for the construct expression. It will set the
   * if the isVisible_ parameter to false if the construct expression is not
   * clickable for some reason.
   * 
   * @param ce_ a construct expression (must be not null).
   * @param isVisible_ see description for details.
   * @return a constructor decl or null on error.
   */
  const clang::CXXConstructorDecl* getCtorDecl(
    const clang::CXXConstructExpr* ce_,
    bool& isVisible_)
  {
    isVisible_ = false;
    
    const clang::CXXConstructorDecl* ctor = ce_->getConstructor();
    if (!ctor || ctor->isImplicit())
    {
      return ctor;
    }

    std::string relevantText;
    if (!getSourceForRange(ce_->getSourceRange(), relevantText))
    {
      return ctor;
    }

    if (relevantText.find(ctor->getNameAsString()) == std::string::npos)
    {
      // The constructor is not presents in the source code, so we should not
      // save it.
      return ctor;
    }
    
    isVisible_ = true;
    return ctor;
  }

  struct TypeLocState
  {
    bool inVar = false;
    bool inParmVar = false;
    bool inField = false;
    bool inFunctionProto = false;
    bool inCompoundStmt = false;

    model::CppAstNode::AstType getTypeLocType()
    {
      if (inParmVar)
        return model::CppAstNode::AstType::ParameterTypeLoc;

      if (inField)
        return model::CppAstNode::AstType::FieldTypeLoc;

      if (inFunctionProto)
        return model::CppAstNode::AstType::ReturnTypeLoc;

      if (inCompoundStmt)
        return model::CppAstNode::AstType::LocalTypeLoc;

      if (inVar)
        return model::CppAstNode::AstType::GlobalTypeLoc;

      return model::CppAstNode::AstType::TypeLocation;
    }

  };

  /**
   * Gets the source code for a source range using the source manager. On
   * any failure the src_ parameter is not modified.
   *
   * @param range_ the source range.
   * @param src_ an output string for the source code.
   * @return ture on success, fasle on fail.
   */
  bool getSourceForRange(
    const clang::SourceRange& range_,
    std::string& src_)
  {
    if (range_.isInvalid())
    {
      return false;
    }
  
    clang::SourceLocation beg = _clangSrcMgr.getSpellingLoc(range_.getBegin());
    clang::SourceLocation end = _clangSrcMgr.getSpellingLoc(range_.getEnd());
    if (beg.isInvalid() || end.isInvalid())
    {
      SLog(cc::util::DEBUG)
        << "Wrong spelling locations from good locations!";
      return false;
    }
  
    auto begDSL = _clangSrcMgr.getDecomposedSpellingLoc(beg);
    auto endDSL = _clangSrcMgr.getDecomposedSpellingLoc(end);
  
    // Check FileId
    if (begDSL.first != endDSL.first)
    {
      // Start and End location is in different file/buffer !
      return false;
    }
  
    // Check offset
    if (begDSL.second > endDSL.second)
    {
      SLog(cc::util::DEBUG)
        << "Wrong file offsets in a source range!";
      return false;
    }
  
    bool srcDataInvalid = true;
    const char* srcData = _clangSrcMgr.getCharacterData(beg, &srcDataInvalid);
    if (srcDataInvalid || !srcData)
    {
      // This could happen
      return false;
    }
  
    std::size_t endOffset = endDSL.second + clang::Lexer::MeasureTokenLength(
      range_.getEnd(), _clangSrcMgr, _session.astContext.getLangOpts());
    src_.assign(srcData, (endOffset - begDSL.second) + 1);
    return true;
  }

  std::shared_ptr<model::Workspace> _w;
  SourceManager& _srcMgr;
  std::vector<model::CppAstNodePtr> _aststack;
  const clang::SourceManager& _clangSrcMgr;
  SymbolHelper _symbolHelper;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>& _newNodes;
  std::vector< std::unordered_set< const void* > > _writtenNodes;
  CxxAstPersister& _astPersister;
  CxxParseSession& _session;
  std::vector<model::CppPointerAnalysis> _pointerAnalysis;

  TypeLocState tlState;
};

} // parser
} // cc

#endif
