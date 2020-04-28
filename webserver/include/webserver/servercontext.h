#ifndef CC_WEBSERVER_SERVERCONTEXT_H
#define CC_WEBSERVER_SERVERCONTEXT_H

#include <memory>

#include <boost/program_options.hpp>

namespace cc
{
namespace webserver
{

class SessionManager;

/**
 * ServerContext is given to each service request handler's constructor. This
 * class can be used by the handlers to obtain information about the running
 * server's configuration.
 */
struct ServerContext
{
  ServerContext(const std::string& compassRoot_,
                const boost::program_options::variables_map& options_)
    : ServerContext(compassRoot_, options_, nullptr)
  {
  }

  ServerContext(const std::string& compassRoot_,
                const boost::program_options::variables_map& options_,
                SessionManager* sessionManager_)
    : compassRoot(compassRoot_), options(options_),
      sessionManager(sessionManager_)
  {
  }

  /**
   * The directory where the CodeCompass server was installed to.
   */
  const std::string& compassRoot;
  /**
   * The command-line invocation options of the server, after parsing.
   */
  const boost::program_options::variables_map& options;
  /**
   * The session manager object the server is using to handle users and
   * authentications.
   *
   * This is an opaque pointer to a type internal to the Web server
   * implementation. Clients may instantiate SessionManagerAccess from
   * webserver/session.h to interface with the SessionManager.
   */
  SessionManager* sessionManager;
};

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_SERVERCONTEXT_H
