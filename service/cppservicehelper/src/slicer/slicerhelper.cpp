#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <reparser/reparser.h>
#include <cxxparser/internal/filelocutilbase.h>
#include <core-api/common_types.h>

#include "slicerhelper.h"

namespace cc
{ 
namespace service
{  
namespace language
{

class Finder : public clang::RecursiveASTVisitor<Finder>
{  
public:
  Finder(const core::FilePosition& filePos_, model::FilePtr file_, clang::SourceManager& srcMgr_) : 
    filePos(filePos_), file(file_), target(nullptr), floc(srcMgr_), srcMgr(srcMgr_) {}
  
  bool VisitFunctionDecl(clang::FunctionDecl *decl)
  {
    auto startLoc = decl->getLocStart();    
    if (srcMgr.isMacroArgExpansion(startLoc))
        startLoc = srcMgr.getSpellingLoc(startLoc);
    
    model::Position::postype sline = 0;
    model::Position::postype scol = 0;    
    floc.getStartLineInfo(startLoc, sline, scol);

    
    auto endLoc = decl->getLocEnd();
    if (srcMgr.isMacroArgExpansion(endLoc))
        startLoc = srcMgr.getSpellingLoc(startLoc);
    
    model::Position::postype eline = 0;
    model::Position::postype ecol = 0;    
    floc.getStartLineInfo(endLoc, eline, ecol);       
    
    std::string path;
    floc.getFilePath(startLoc, path);
    /*
    std::cerr << "DEBUG: current function pos: " << decl-> getNameAsString() 
              << " " << sline << ", " << scol << " -- " 
              << eline << ", " << ecol << std::endl;
    //*/
    if( (path == file->path) && (sline <= filePos.pos.line) && (filePos.pos.line <= eline))
    {
      target = decl;
      return false; //found it, don't need further traversal
    }
    
    return true;
  }
  
  clang::Decl* result() { return target; }
  
private:
  const core::FilePosition& filePos;  
  model::FilePtr file;
  clang::Decl* target;
  parser::FileLocUtilBase floc;
  clang::SourceManager& srcMgr;
};  

void SlicerHelper::getBackwardSlicePos(std::vector<core::Range>& _return, const core::FilePosition& filePos)
{
  PDG pdg(db);
  buildPDG(pdg, filePos);
  pdg.backwardSlicePositions(_return, filePos);
}

void SlicerHelper::getForwardSlicePos(std::vector<core::Range>& _return, const core::FilePosition& filePos)
{
  PDG pdg(db);
  buildPDG(pdg, filePos);
  pdg.forwardSlicePositions(_return, filePos);
}

void SlicerHelper::buildPDG(PDG& pdg, const core::FilePosition& filePos)
{
  
  model::FilePtr file;
  {
    odb::transaction t(db->begin());    
    typedef odb::query<model::File>  FQuery;   

    auto res (db->query<model::File>(
      FQuery::id == std::stoull(filePos.file.fid)
    )); 
  
    file.reset( new model::File(*res.begin()));
  }
    
  cc::parser::Reparser reparser(db, virtualFileStorage);
  auto ast = reparser.createAst(file);
  
  Finder visitor(filePos, file, ast->getASTContext().getSourceManager());
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
  
  std::cerr << "DEBUG Finder!! : result = " << visitor.result() << std::endl;
  
  pdg.build(visitor.result(), &(ast->getASTContext().getSourceManager()));
  
}


} // language
} // service
} // cc
