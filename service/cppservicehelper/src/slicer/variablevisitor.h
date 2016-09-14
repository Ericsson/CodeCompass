#ifndef SERVICE_CPPSERVICEHELPER_SLICER_VARIABLE_VISITOR_H
#define SERVICE_CPPSERVICEHELPER_SLICER_VARIABLE_VISITOR_H

#include <string>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <cxxparser/internal/filelocutilbase.h>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/position.h>
#include <model/asttype.h>
#include <cppservicehelper/cppservicehelper.h>
#include <util/odbtransaction.h>

#include "pdg.h"

namespace cc
{ 
namespace service
{  
namespace language
{

  
/**
 * This is a helper class, which computes whether the variable is read or written. Mainly it receives the info from
 * the database, but for increment, decrement and the left argument of compound operators the use state is also added.
 */
class VariableVisitor : public clang::RecursiveASTVisitor<VariableVisitor>
{
public:
  enum Behaviour { Automatic, ForcedDef, ForcedUse };
  VariableVisitor(std::shared_ptr<odb::database> db_, clang::SourceManager& srcMgr_, Behaviour b = Automatic) : 
    db(db_), cppServiceHelper(db_), transaction(db_), srcMgr(srcMgr_), floc(srcMgr_), behaviour(b) {}
  
  bool VisitVarDecl(clang::VarDecl* varDecl)
  {
    std::string filename;
    floc.getFilePath(varDecl->getLocStart(), filename);
    
    model::Position::postype line;
    model::Position::postype col;
    
    if(floc.getStartLineInfo(varDecl->getLocStart(), line, col))
    {
      model::CppAstNode astNode = getVariableByPosition(line, col, filename);
      workingPDGNode->defs.insert(astNode.mangledNameHash);
    }
    
    return true;
  }
  
  bool VisitDeclRefExpr(clang::DeclRefExpr* decl)
  {
    
    std::string filename;
    floc.getFilePath(decl->getLocStart(), filename);
    
    model::Position::postype line;
    model::Position::postype col;
    
    if(floc.getStartLineInfo(decl->getLocStart(), line, col))
    {
      //if the variable has no position we skip it        
      model::CppAstNode astNode = getVariableByPosition(line, col, filename);
      if(behaviour == ForcedDef)
        workingPDGNode->defs.insert(astNode.mangledNameHash);
      else if (behaviour == ForcedUse)
        workingPDGNode->uses.insert(astNode.mangledNameHash);
      else if( behaviour == Automatic)
      {
        if(astNode.astType == model::CppAstNode::AstType::Read)
          workingPDGNode->uses.insert(astNode.mangledNameHash);
        else if(astNode.astType == model::CppAstNode::AstType::Write ||
                astNode.astType == model::CppAstNode::AstType::Definition)
          workingPDGNode->defs.insert(astNode.mangledNameHash);      
      }
    }    

    return true;
  }
  
  bool VisitBinaryOperator(clang::BinaryOperator* bop)
  {    
    if(bop->isCompoundAssignmentOp())
    {
      VariableVisitor vv(db, srcMgr, ForcedUse);
      vv.setWorkingPDGNode(workingPDGNode);
      vv.TraverseStmt(bop->getLHS());
    }    
    return true;
  }
  
  bool VisitUnaryOperator(clang::UnaryOperator* uop)
  {
    if(uop->isIncrementDecrementOp())
    {
      VariableVisitor vv(db, srcMgr, ForcedUse);
      vv.setWorkingPDGNode(workingPDGNode);
      vv.TraverseStmt(uop->getSubExpr());
    }
    return true;
  }
  
  void setWorkingPDGNode(Node* n)
  {
    workingPDGNode = n;
  }
  
private:
  model::FileId getFileId(const std::string& fn)
  {
    //std::cerr << "DEBUG: filename " << fn << std::endl;
    odb::transaction t(db->begin());

    typedef odb::query<model::File>  FQuery;

    auto res (db->query<model::File>(
      FQuery::path == fn
    ));
    
    return (*res.begin()).id;
  }
  
  model::CppAstNode getVariableByPosition(int line, int col, const std::string& fn)
  {
    
    core::FileId fid;    
    fid.__set_fid(std::to_string(getFileId(fn)));
        
    core::Position pos;
    pos.__set_line(line);
    pos.__set_column(col);
    
    core::FilePosition fp;
    fp.__set_file(fid);
    fp.__set_pos(pos);
    
    return transaction([&, this]()
    {
      return cppServiceHelper.selectProperAstNode(
        cppServiceHelper.queryAstNodesByPosition(fp, std::vector<std::string>()));      
    });
    
    return model::CppAstNode();
  }
  
  std::shared_ptr<odb::database> db;
  CppServiceHelper cppServiceHelper;
  util::OdbTransaction transaction;
  clang::SourceManager& srcMgr;
  parser::FileLocUtilBase floc;
  Behaviour behaviour;
  
  Node* workingPDGNode;
  
};
  
} // language
} // service
} // cc
  
#endif
