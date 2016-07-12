#ifndef CC_MONGOOSE_MAINREQUESTHANDLER_H
#define CC_MONGOOSE_MAINREQUESTHANDLER_H

//#include <mongoose/user_stat_sender.h>
#include <plugin/pluginhandler.h>
#include <mongoose/plugin.h>
#include <mongoose/authentication.h>

namespace cc
{
namespace mongoose
{
  
struct MainRequestHandler
{
public:
  enum class AuthMode { NONE, DIGEST, COOKIE, LDAP };

  plugin::PluginHandler<RequestHandler>      pluginHandler = RequestHandler::version;
  std::string                        documentRoot;
  std::string                        digestPasswdFile;
  AuthMode                           authMode = AuthMode::NONE;
  //std::shared_ptr<UserStat>          userStat; 
  //std::shared_ptr<UserStatSender>    userStatSender;
  std::shared_ptr<Authentication>    auth;
  std::map<std::string, std::string> dataDir;

public:
  int begin_request_handler(struct mg_connection *conn);
  int operator()(struct mg_connection *conn, enum mg_event ev);  
private:
  std::string getDocDirByURI(std::string uri);
};
  
} // mongoose
} // cc

#endif // CC_MONGOOSE_MAINREQUESTHANDLER_H