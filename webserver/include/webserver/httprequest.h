#ifndef CC_WEBSERVER_HTTPREQUEST_H
#define CC_WEBSERVER_HTTPREQUEST_H

#include <arpa/inet.h>
#include <string>
#include <utility>
#include <vector>

struct mg_connection;

namespace cc
{
namespace webserver
{

/**
 * Contains the parsed values of an incoming HTTP request as understood by the
 * server. Unlike Mongoose's buffers, this record owns its resources directly.
 */
struct HTTPRequest
{
  struct mg_connection* connection;

#if MG_ENABLE_IPV6 == 1
  // '[' + sizeof(ipv6-address) + ']' + strlen(":12345")
  char address[1 + INET6_ADDRSTRLEN + 1 + 6];
#else
  // sizeof("255.255.255.255") + strlen(":12345")
  char address[INET_ADDRSTRLEN + 6];
#endif

  char protocol[9]; // E.g. HTTP/1.1
  char method[8];   // Longest HTTP methods are OPTIONS and CONNECT.
  std::string uri;
  std::string query;
  std::vector<std::pair<std::string, std::string>> headers;
  std::string body;

  /**
   * Returns the value for the given HTTP Header in the current request.
   */
  const char* getSpecificHeader(const char* headerName_) const
  {
    for (const auto& header : headers)
      if (header.first == headerName_)
        return header.second.c_str();
    return nullptr;
  }
};

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_HTTPREQUEST_H
