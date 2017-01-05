#ifndef CODECOMPASS_CPPSERVICEHELPER_REPARSER_H
#define CODECOMPASS_CPPSERVICEHELPER_REPARSER_H

#include <vector>
#include <string>

#include <memory>
#include <model/file.h>

#include "odb/database.hxx"

namespace clang
{
  class ASTUnit;
  namespace tooling
  {
    class ClangTool;
  }
}

namespace cc
{
namespace parser
{

/**
 * This class is responsible to reparse one compilation unit.
 * It reads the compiling options from database, load the compilation unit and the required
 * header files and reparse it.
 */
class Reparser
{
public:
  Reparser(std::shared_ptr<odb::database> db_, std::map<std::string, std::string>& virtualFileStorage_)
    :db(db_), virtualFileStorage(virtualFileStorage_) {}
  
  std::unique_ptr<clang::ASTUnit> createAst(model::FilePtr);
private:
  void readCompilerOptions(std::vector<std::string>& opts, model::FilePtr);
  void mapIncludedHeaders(model::FilePtr, clang::tooling::ClangTool&);
  void loadFile(model::FileId, clang::tooling::ClangTool&);
  void loadFileContent(model::FilePtr file, std::string& result);
   
  std::shared_ptr<odb::database> db;
  std::map<std::string, std::string>& virtualFileStorage;
};
  


} // parser
} // cc

#endif
