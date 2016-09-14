#include <vector>
#include <deque>
#include <set>
#include <iostream> 


#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/raw_os_ostream.h>

#include <model/buildparameter.h>
#include <model/buildparameter-odb.hxx>
#include <model/buildsource.h>
#include <model/buildsource-odb.hxx>
#include <model/file.h>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>
#include <model/cxx/cppheaderinclusion.h>
#include <model/cxx/cppheaderinclusion-odb.hxx>

#include "reparser/reparser.h"

namespace cc
{
namespace parser
{

std::unique_ptr<clang::ASTUnit> Reparser::createAst(model::FilePtr file)
{
  odb::transaction t(db->begin());
  std::vector<std::string> opts;
  readCompilerOptions(opts, file);  
  
  std::string content;
  loadFileContent(file, content);
  
  clang::tooling::FixedCompilationDatabase ccCompDb(".", opts);
  clang::tooling::ClangTool ccCompTool(ccCompDb, llvm::ArrayRef<std::string>(file->path));
  llvm::raw_os_ostream os(std::cerr);
  auto diagnosticPrinter = new clang::TextDiagnosticPrinter(os, new clang::DiagnosticOptions());
  diagnosticPrinter->setPrefix("****** error in parsing file: ");
  ccCompTool.setDiagnosticConsumer(diagnosticPrinter);    
  
  auto res = virtualFileStorage.insert(std::make_pair(file->path, content));
  ccCompTool.mapVirtualFile(res.first->first, res.first->second);
    
  mapIncludedHeaders(file, ccCompTool);
  std::vector<std::unique_ptr<clang::ASTUnit>> asts;
  ccCompTool.buildASTs(asts);
  return std::move(asts.front());
}

void Reparser::readCompilerOptions(std::vector<std::string>& opts, model::FilePtr file)
{
  typedef odb::query<model::BuildSource>  BSQuery;   
  typedef odb::query<model::BuildParameter>  BPQuery;  

  auto res (db->query<model::BuildSource>(
    BSQuery::file == file->id
  ));      
  
  auto actionid = res.begin()->action->id;
  
  auto resOpts (db->query<model::BuildParameter>(
    BPQuery::action == actionid
  )); 
  
  for( auto param : resOpts )
  {
    opts.push_back(param.param);
  }  
}

void Reparser::mapIncludedHeaders(model::FilePtr file, clang::tooling::ClangTool& ccCompTool)
{
  
  std::set<model::FileId> to_visit{file->id};
  std::set<model::FileId> visited;  
  
  while(!to_visit.empty())
  {
    model::FileId current = *to_visit.begin();        
    
    std::set<model::FileId> ids_to_save;            

    typedef odb::query<model::CppHeaderInclusion>  HIQuery;   

    auto res (db->query<model::CppHeaderInclusion>(
      HIQuery::includer == current
    ));           
    
    //std::cerr << "includer: " << current << ", includes: ";
    for(auto row : res)
    {
      model::FileId id = row.included.load()->id;
      if(visited.count(id) == 0)
      {
        //std::cerr << static_cast<int64_t>(id) << " ";
        to_visit.insert(id);   
        ids_to_save.insert(id);
      }
    }
    //std::cerr << std::endl;
      
    
    for( auto cid : ids_to_save)
      loadFile(cid, ccCompTool);      
        
    to_visit.erase(current);
    visited.insert(current);
  }
}

void Reparser::loadFile(model::FileId id, clang::tooling::ClangTool& clangTool)
{
  //std::cerr << "\nLook for file: " << static_cast<int64_t>(id) << std::endl;
  model::FilePtr file;
   
  typedef odb::query<model::File>  FQuery;   

  auto qres (db->query<model::File>(
    FQuery::id == id
  )); 
  
  
  file.reset( new model::File(*qres.begin()));    
  
  std::string content;
  loadFileContent(file, content);
  auto res = virtualFileStorage.insert(std::make_pair(file->path, content));
  clangTool.mapVirtualFile(res.first->first, res.first->second);
}

void Reparser::loadFileContent(model::FilePtr file, std::string& result)
{
  typedef odb::query<model::FileContent>  FCQuery;   

  auto res (db->query<model::FileContent>(
    FCQuery::hash == file->content.load()->hash
  )); 
  
  result = res.begin()->content;
  result += "\0";
}



} // parser
} // cc
