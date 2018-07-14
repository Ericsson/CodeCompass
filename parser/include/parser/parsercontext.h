#ifndef CC_PARSER_PARSERCONTEXT_H
#define CC_PARSER_PARSERCONTEXT_H

#include <memory>
#include <boost/program_options.hpp>
#include <odb/database.hxx>

namespace po = boost::program_options; 

namespace cc
{  
namespace parser
{

class SourceManager;

struct ParserContext 
{  
  ParserContext(
    std::shared_ptr<odb::database> db_,
    SourceManager& srcMgr_,
    std::string& compassRoot_,
    po::variables_map& options_) :
      db(db_),
      srcMgr(srcMgr_),
      compassRoot(compassRoot_),
      options(options_)
  {
  }

  std::shared_ptr<odb::database> db;
  SourceManager& srcMgr;
  std::string& compassRoot;
  po::variables_map& options;
};

} // parser
} // cc

#endif	// CC_PARSER_PARSERCONTEXT_H

