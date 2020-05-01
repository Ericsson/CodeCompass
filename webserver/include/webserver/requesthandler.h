#ifndef CC_WEBSERVER_REQUESTHANDLER_H
#define CC_WEBSERVER_REQUESTHANDLER_H

#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "httprequest.h"
#include "pluginhandler.h"

namespace cc
{ 
namespace webserver
{

class RequestHandler
{
public:
  virtual std::string key() const = 0;
  virtual std::string beginRequest(const HTTPRequest& req_) = 0;
  virtual ~RequestHandler() = default;
};

} // webserver
} // cc

#endif // CC_WEBSERVER_REQUESTHANDLER_H
