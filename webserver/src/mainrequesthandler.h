#ifndef CC_WEBSERVER_MAINREQUESTHANDLER_H
#define CC_WEBSERVER_MAINREQUESTHANDLER_H

#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>

namespace cc
{
namespace webserver
{

class Session;
class SessionManager;

class MainRequestHandler
{
public:
  SessionManager* sessionManager;
  PluginHandler<RequestHandler> pluginHandler;
  std::map<std::string, std::string> dataDir;
  std::string gaTrackingIdPath;

  int operator()(struct mg_connection* conn_, enum mg_event ev_);

private:
  int begin_request_handler(struct mg_connection* conn_);
  std::string getDocDirByURI(std::string uri_);

  // Detail template - implementation in the .cpp only.
  template <typename F>
  auto executeWithSessionContext(cc::webserver::Session* sess_, F func);
};

} // webserver
} // cc

#endif // CC_WEBSERVER_MAINREQUESTHANDLER_H
