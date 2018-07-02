#ifndef CC_PARSER_PARSERCONTEXT_H
#define CC_PARSER_PARSERCONTEXT_H

#include <memory>
#include <unordered_map>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <odb/database.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/hash.h>
#include <util/odbtransaction.h>

namespace po = boost::program_options; 

namespace cc
{  
namespace parser
{

class SourceManager;

 /**
 * Defines file status categories for incremental parsing.
 */
enum class IncrementalStatus
{
  MODIFIED,
  ADDED,
  DELETED
};

struct ParserContext 
{  
  ParserContext(
      std::shared_ptr<odb::database> db_,
      SourceManager& srcMgr_,
      std::string& compassRoot_,
      po::variables_map& options_,
      std::unordered_map<std::string, IncrementalStatus> fileStatus_);

  std::shared_ptr<odb::database> db;
  SourceManager& srcMgr;
  std::string& compassRoot;
  po::variables_map& options;
  std::unordered_map<std::string, IncrementalStatus> fileStatus;
};

} // parser
} // cc

#endif	// CC_PARSER_PARSERCONTEXT_H

