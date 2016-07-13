#ifndef CC_MONGOOSE_MAINREQUESTHANDLER_H
#define CC_MONGOOSE_MAINREQUESTHANDLER_H

#include <plugin/pluginhandler.h>
#include <mongoose/plugin.h>

namespace cc
{
namespace mongoose
{
  
struct MainRequestHandler
{
public:
  enum class AuthMode { NONE, DIGEST, COOKIE, LDAP };

  plugin::PluginHandler<RequestHandler> pluginHandler = RequestHandler::version;
  std::string                           documentRoot;
  std::string                           digestPasswdFile;
  AuthMode                              authMode = AuthMode::NONE;
  std::map<std::string, std::string>    dataDir;

public:
  int begin_request_handler(struct mg_connection *conn_);
  int operator()(struct mg_connection *conn_, enum mg_event ev_);  
private:
  std::string getDocDirByURI(std::string uri_);
};
  
} // mongoose
} // cc

#endif // CC_MONGOOSE_MAINREQUESTHANDLER_H