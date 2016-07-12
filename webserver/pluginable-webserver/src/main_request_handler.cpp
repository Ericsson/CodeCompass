#include <mongoose/main_request_handler.h>
#include <mongoose/mongoose_utility.h>

namespace cc
{
namespace mongoose
{

int MainRequestHandler::begin_request_handler(struct mg_connection *conn)
{
  //--- If not authenticated yet ---//  

  std::pair<std::string, std::string> userData;

  if (authMode == AuthMode::LDAP &&
    !auth->isAuthenticated(getAuthToken(conn)))
  {
    userData = getUsernamePassword(conn);
    std::string authtoken;

    if (!userData.first.empty())
      authtoken = auth->authenticate(userData.first, userData.second);

    if (authtoken.empty())
    {
      std::string loginPage = getLoginPage(documentRoot);

      mg_send_header(conn, "Content-Type", "text/html");
      mg_send_data(conn, loginPage.c_str(), loginPage.size());

      return MG_TRUE;
    }
    else
    {
      mg_send_header(conn, "Set-Cookie", (std::string("authtoken=") + authtoken).c_str());
    }
  }
  else if (auth && !auth->isAuthenticated(getAuthToken(conn)) && conn->content_len != 0)
  {
    mg_send_header(conn, "Set-Cookie",
      (std::string("authtoken=") + auth->authenticate(getRemoteIp(conn), "")).c_str());
  }

  //--- If already authenticated ---//

  std::string uri = conn->uri + 1; // We advance it by one because of
                                   // the '/' character

//  SLog(cc::util::INFO)
//    << getCurrentDate() << " Connection from " << conn->remote_ip
//    << ':' << conn->remote_port << " requested URI: " << uri
//    << std::endl;

  auto handler = pluginHandler.getImplementation(uri);
  if (handler)
  {
    //std::shared_ptr<UserStat>          userStat; 
    return handler->beginRequest(conn);
  }

  if (uri.find_first_of("doxygen/") == 0)
  {
    mg_send_file(conn, getDocDirByURI(uri).c_str());
    return MG_MORE;
  }

  // Returning MG_FALSE tells mongoose that we didn't served the request
  // so mongoose should serve it
  return MG_FALSE;
}

int MainRequestHandler::operator()(struct mg_connection *conn, enum mg_event ev)
{
  int result;

  switch (ev)
  {
    case MG_REQUEST:
      return begin_request_handler(conn);

    case MG_AUTH:
    {
      if (digestPasswdFile.empty())
        return MG_TRUE;

      FILE* fp = fopen(digestPasswdFile.c_str(), "r");
      if (fp) {
        result = mg_authorize_digest(conn, fp);
        fclose(fp);
        return result;
      } else {
//        SLog(cc::util::ERROR)
//          << "Password file could not be opened: " << digestPasswdFile;
        //throw an exception instead of a segfault/reauth
        //an internal server error response would be nicer
        throw std::runtime_error("Password file could not be opened.");
      }
    }

    default:
      break;
  }

  return MG_FALSE;
}

std::string MainRequestHandler::getDocDirByURI(std::string uri)
{
  if (uri.empty())
    return "";

  if (uri.back() == '/')
    uri.pop_back();

  std::size_t pos1 = uri.find('/');
  std::size_t pos2 = uri.find('/', pos1 + 1);

  std::string ws = uri.substr(pos1 + 1, pos2 - pos1 - 1);

  std::string file;
  if (pos2 != std::string::npos)
    file = uri.substr(pos2);

  return dataDir[ws] + "/docs" + file;
}

}
}
