#ifndef CC_PARSER_PARSERCONTEXT_H
#define CC_PARSER_PARSERCONTEXT_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>
#include <odb/database.hxx>

namespace po = boost::program_options; 

namespace cc
{  
namespace parser
{

class SourceManager;

/**
 * Defines file status categories for incremental parsing.
 *
 * State in database VERSUS state on disk at parse time.
 */
enum class IncrementalStatus
{
  MODIFIED,
  ADDED,
  DELETED,
  ACTION_CHANGED
};

struct ParserContext 
{  
  ParserContext(
    std::shared_ptr<odb::database> db_,
    SourceManager& srcMgr_,
    std::string& compassRoot_,
    po::variables_map& options_);

  std::shared_ptr<odb::database> db;
  SourceManager& srcMgr;
  std::string& compassRoot;
  po::variables_map& options;
  std::unordered_map<std::string, IncrementalStatus> fileStatus;
  std::vector<std::string> moduleDirectories;
};

} // parser
} // cc

#endif	// CC_PARSER_PARSERCONTEXT_H

