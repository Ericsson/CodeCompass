#ifndef CC_WEBSERVER_PLUGIN_H
#define CC_WEBSERVER_PLUGIN_H

#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "pluginhandler.h"
#include "mongoose.h"

namespace cc 
{ 
namespace webserver
{

class RequestHandler
{
public:
  virtual std::string key() const = 0;
  virtual int beginRequest(struct mg_connection*) = 0;
  virtual ~RequestHandler() {}
};

} // webserver
} // cc

#endif
