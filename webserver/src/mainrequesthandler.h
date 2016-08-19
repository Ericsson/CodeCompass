#ifndef CC_WEBSERVER_MAINREQUESTHANDLER_H
#define CC_WEBSERVER_MAINREQUESTHANDLER_H

#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>

namespace cc
{
namespace webserver
{
  
struct MainRequestHandler
{
public:
  PluginHandler<RequestHandler> pluginHandler;
  std::string digestPasswdFile;
  std::map<std::string, std::string> dataDir;

  int operator()(struct mg_connection* conn_, enum mg_event ev_);

private:
  int begin_request_handler(struct mg_connection *conn_);
  std::string getDocDirByURI(std::string uri_);
};
  
} // webserver
} // cc

#endif // CC_WEBSERVER_MAINREQUESTHANDLER_H
