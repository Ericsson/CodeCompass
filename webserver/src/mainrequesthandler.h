#ifndef CC_WEBSERVER_MAINREQUESTHANDLER_H
#define CC_WEBSERVER_MAINREQUESTHANDLER_H

#include <string>
#include <vector>

#include <webserver/httprequest.h>
#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>

namespace cc
{
namespace webserver
{

class Session;
class SessionManager;

struct MainRequestHandler
{
public:
  SessionManager* sessionManager;
  PluginHandler<RequestHandler> pluginHandler;
  std::vector<std::string> servedPaths;

  /**
   * Executes the given request and sends back a reply into the connection where
   * the request originated from.
   */
  void operator()(HTTPRequest& req_);

  void updateServedPathsFromPluginHandler();

private:
  void begin_request_handler(const HTTPRequest& req_);

  // Detail template - implementation in the .cpp only.
  template <typename F>
  auto executeWithSessionContext(cc::webserver::Session* sess_, F func);
};

} // webserver
} // cc

#endif // CC_WEBSERVER_MAINREQUESTHANDLER_H
