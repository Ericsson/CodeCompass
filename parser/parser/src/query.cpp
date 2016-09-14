#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

#include <odb/database.hxx>
#include <odb/transaction.hxx>


#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>

#ifdef DATABASE_SQLITE
#include <odb/sqlite/database.hxx>
#endif
#ifdef DATABASE_PGSQL
#include <odb/pgsql/database.hxx>
#endif

using namespace cc::model;
using namespace cc::util;

int main(int argc, char* argv[])
{
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
    new StandardErrorLogStrategy()));

  if(argc < 2)
  {
    std::cerr << "Usage: parse <options>\n"
              << "  -d, --database dbstring \n\t database name\n";
    return 1;
  }

  std::string dbname = "";

  int i=1;
  while(i < argc)
  {
    if( std::strcmp(argv[i], "-d") == 0 || std::strcmp(argv[i], "--database") == 0 )
      dbname = argv[i+1];

    i += 2;
  }

  std::cout << "db: " << dbname << std::endl;  

#ifdef DATABASE_SQLITE
  std::unique_ptr<odb::core::database> db (new odb::sqlite::database(dbname));
#endif
#ifdef DATABASE_PGSQL
  std::unique_ptr<odb::core::database> db (new odb::pgsql::database(dbname));
#endif
        /*static_cast<int>(args.size()),
        const_cast<char**>(args.data()),
        false,
        SQLITE_OPEN_READWRITE);*/
  
  odb::transaction t(db->begin());
  
  typedef odb::query<CppAstNode>  AQuery;
  typedef odb::result<CppAstNode> AResult;

  AResult res ( db->query<CppAstNode>(
      (AQuery::symbolType == CppAstNode::SymbolType::Function)
  ));
  
  for(AResult::iterator i(res.begin()); i != res.end(); ++i)
  {
    std::cout << i->astValue << std::endl;
  }
  
  AResult res2 ( db->query<CppAstNode>(
      (AQuery::symbolType == CppAstNode::SymbolType::Function) &&
      (AQuery::astType == CppAstNode::AstType::Definition)));
  
  for(AResult::iterator i(res2.begin()); i != res2.end(); ++i)
  {
    std::cout << i->astValue << std::endl;
  }
  
}
