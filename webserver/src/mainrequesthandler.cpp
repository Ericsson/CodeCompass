#include <util/util.h>
#include <util/logutil.h>

#include "mainrequesthandler.h"

namespace cc
{
namespace webserver
{

int MainRequestHandler::begin_request_handler(struct mg_connection* conn_)
{
  // We advance ot by one because of the '/' character.
  const std::string& uri = conn_->uri + 1;

  LOG(debug)
    << util::getCurrentDate() << " Connection from " << conn_->remote_ip
    << ':' << conn_->remote_port << " requested URI: " << uri;

  auto handler = pluginHandler.getImplementation(uri);
  if (handler)
    return handler->beginRequest(conn_);

  if (uri.find_first_of("doxygen/") == 0)
  {
    mg_send_file(conn_, getDocDirByURI(uri).c_str());
    return MG_MORE;
  }

  // Returning MG_FALSE tells mongoose that we didn't served the request
  // so mongoose should serve it.
  return MG_FALSE;
}

int MainRequestHandler::operator()(
  struct mg_connection* conn_,
  enum mg_event ev_)
{
  int result;

  switch (ev_)
  {
    case MG_REQUEST:
      return begin_request_handler(conn_);

    case MG_AUTH:
    {
      if (digestPasswdFile.empty())
        return MG_TRUE;

      FILE* fp = fopen(digestPasswdFile.c_str(), "r");
      if (fp) 
      {
        result = mg_authorize_digest(conn_, fp);
        fclose(fp);
        return result;
      }
      else 
      {
        LOG(error)
          << "Password file could not be opened: " << digestPasswdFile;

        // TODO: An internal server error response would be nicer.
        throw std::runtime_error("Password file could not be opened.");
      }
      break;
    }

    default:
      break;
  }

  return MG_FALSE;
}

std::string MainRequestHandler::getDocDirByURI(std::string uri_)
{
  if (uri_.empty())
    return "";

  if (uri_.back() == '/')
    uri_.pop_back();

  std::size_t pos1 = uri_.find('/');
  std::size_t pos2 = uri_.find('/', pos1 + 1);

  std::string ws = uri_.substr(pos1 + 1, pos2 - pos1 - 1);

  std::string file;
  if (pos2 != std::string::npos)
    file = uri_.substr(pos2);

  return dataDir[ws] + "/docs" + file;
}

}
}
