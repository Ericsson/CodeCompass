#ifndef MONGOOSE_UTILITY_H_
#define MONGOOSE_UTILITY_H_

#include <mongoose/mongoose.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace cc 
{ 
namespace mongoose 
{

const std::string PLUGIN_DIR = "plugin_dir";
const std::string CONFIG_FILE = "config_file";
const std::string HELP = "help";
const std::string LISTENING_PORTS = "listening_port";
const std::string NUM_THREADS = "threads";

void parseConfiguration(
  const boost::program_options::options_description&  options_,
  int argc_, char **argv_,
  boost::program_options::variables_map& varMap_, bool allowUnkown_);

/**
 * This function returns the content part of a http header as string.
 */
std::string getContent(mg_connection *conn_);

std::string getCurrentDate();

} // mongoose 
} // cc

#endif /* MONGOOSE_UTILITY_H_ */
