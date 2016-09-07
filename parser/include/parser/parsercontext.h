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
    po::variables_map& options_) :
      db(db_),
      srcMgr(srcMgr_),
      options(options_)
  {
  }

  std::shared_ptr<odb::database> db;
  SourceManager& srcMgr;
  po::variables_map& options;
};

} // parser
} // cc

#endif	// CC_PARSER_PARSERCONTEXT_H

