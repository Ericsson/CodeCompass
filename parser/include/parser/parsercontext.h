#ifndef CC_PARSER_PARSERCONTEXT_H
#define CC_PARSER_PARSERCONTEXT_H

#include <boost/program_options.hpp>

namespace po = boost::program_options; 

namespace cc
{  
namespace parser
{

class SourceManager;

struct ParserContext 
{  
  ParserContext(SourceManager& srcMgr_, po::variables_map& options_):
    srcMgr(srcMgr_), options(options_) {}
     
  SourceManager& srcMgr;
  po::variables_map& options;
};

} // parser
} // cc

#endif	// CC_PARSER_PARSERCONTEXT_H

