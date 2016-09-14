#include <iostream>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/Utils.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include <clang/AST/Mangle.h>
#include <clang/AST/GlobalDecl.h>


class AstPrinter : public clang::RecursiveASTVisitor<AstPrinter>
{
public:
  AstPrinter(clang::SourceManager& sm_) : indentlvl(0), srcMan(sm_) {}
  bool TraverseTypeLoc( clang::TypeLoc TL_ )
  {
    clang::QualType T = TL_.getType();
    const clang::Type *type = T.getTypePtrOrNull();  //operator* of T is overloaded to return a Type&
    if(!type) return true;


    indent(); std::cout << type->getTypeClassName();printPos(TL_); std::cout << std::endl;
    ++indentlvl;
    bool b = clang::RecursiveASTVisitor<AstPrinter>::TraverseTypeLoc(TL_);
    --indentlvl;

    return b;
  }

  bool TraverseDecl( clang::Decl *d_ )
  {
    if (!d_)
    {
      return clang::RecursiveASTVisitor<AstPrinter>::TraverseDecl(d_);
    }

    indent(); std::cout << d_->getDeclKindName(); printPos(d_); std::cout << std::endl;
    ++indentlvl;
    bool b = clang::RecursiveASTVisitor<AstPrinter>::TraverseDecl(d_);
    --indentlvl;

    return b;
  }

  bool TraverseStmt( clang::Stmt *s_ )
  {
    if(!s_) return true;

    indent(); std::cout << s_->getStmtClassName(); printPos(s_); std::cout << std::endl;

    ++indentlvl;
    bool b = clang::RecursiveASTVisitor<AstPrinter>::TraverseStmt(s_);
    --indentlvl;

    return b;
  }

  bool VisitCXXRecordDecl( clang::CXXRecordDecl *d)
  {
    indent();
    std::cout << d->getName().str() << std::endl;

    return true;
  }

private:
  void indent()
  {
    for(int i=0; i<indentlvl; ++i)
    {
      std::cout << "  ";
    }
  }
  
  void printPos(clang::TypeLoc node)
  {
    
    const clang::SourceLocation& startLoc = node.getBeginLoc();
    const clang::SourceLocation& endLoc = node.getEndLoc();

    std::cout << " [TL " << node.getType().getAsString() << "] ";
    printPos(startLoc, endLoc);
  }
  
  template<typename T>
  void printPos(T* node)
  {    
    const clang::SourceLocation& startLoc = node->getLocStart();
    const clang::SourceLocation& endLoc = node->getLocEnd();     
    printPos(startLoc, endLoc);    
  }
  
  void printPos(const clang::SourceLocation& startLoc, const clang::SourceLocation& endLoc)
  {
    std::cout << " ( ";
    printLoc(startLoc, false);
    std::cout << " -- ";
    printLoc(endLoc, true);
    std::cout << " )";
  }
  
  void printLoc(const clang::SourceLocation& loc_, bool isEnd)
  {
    if (loc_.isInvalid()) return;

    
    std::pair<clang::FileID, unsigned> decLoc = srcMan.getDecomposedSpellingLoc(loc_);
    if (decLoc.first.isInvalid()) return;

    bool isInvalid = false;
    unsigned int line = srcMan.getLineNumber(decLoc.first, decLoc.second, &isInvalid);
    if (isInvalid) return;
      
    unsigned int col = srcMan.getColumnNumber(decLoc.first, decLoc.second,&isInvalid);
    if (isInvalid) return;


    if(isEnd)
    {
      clang::LangOptions langOpts;
      col += clang::Lexer::MeasureTokenLength(loc_, srcMan, langOpts);
    }
    
    std::cout << line << ":" << col;
  }
  
  int indentlvl;
  clang::SourceManager& srcMan;

};




int main(int argc, char* argv[])
{
  llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diags =
    clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions());

  clang::CompilerInvocation *ci = new clang::CompilerInvocation();
  clang::CompilerInvocation::CreateFromArgs(*ci, argv+1, argv+argc, *diags);
  ci->setLangDefaults(*(ci->getLangOpts()),
                      clang::IK_CXX,
                      clang::LangStandard::lang_gnucxx11);
  ci->getLangOpts()->Exceptions = 1;
  ci->getLangOpts()->CXXExceptions = 1;
  ci->getPreprocessorOutputOpts().ShowComments = 1;

  clang::ASTUnit* AST = clang::ASTUnit::LoadFromCompilerInvocationAction(ci, diags);
  AstPrinter visitor(AST->getASTContext().getSourceManager());
  visitor.TraverseDecl(AST->getASTContext().getTranslationUnitDecl());
}
