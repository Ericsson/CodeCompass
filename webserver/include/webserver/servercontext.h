#ifndef CC_WEBSERVER_SERVERCONTEXT_H
#define CC_WEBSERVER_SERVERCONTEXT_H

#include <memory>

#include <boost/program_options.hpp>

namespace po = boost::program_options; 

namespace cc
{  
namespace webserver
{

template <class Base>
class PluginHandler;

struct ServerContext
{
  ServerContext(
    const std::string& compassRoot_,
    const po::variables_map& options_) :
      compassRoot(compassRoot_),
      options(options_)
  {
  }

  const std::string& compassRoot;
  const po::variables_map& options;
};

} // webserver
} // cc

#endif	// CC_WEBSERVER_SERVERCONTEXT_H

