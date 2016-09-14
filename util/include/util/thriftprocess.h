#ifndef __CODECOMPASS_UTIL_THRIFT_PROCESS_H__
#define __CODECOMPASS_UTIL_THRIFT_PROCESS_H__

#include <cstdint>
#include <string>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <thread>

#include <boost/make_shared.hpp>
#include <boost/make_unique.hpp>
#include <boost/pointer_cast.hpp>

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>

namespace cc
{
namespace util
{

/**
 * Helper class for starting a process with a socket connection.
 */
class ThriftProcess
{
typedef apache::thrift::transport::TSocket Socket;

public:
  /**
   * Exception class.
   */
  class Failure : public std::runtime_error
  {
  public:
    Failure(const std::string& msg_);
  };

public:
  ThriftProcess(const ThriftProcess&) = delete;
  ThriftProcess& operator = (const ThriftProcess&) = delete;

  /**
   * Inits the members.
   */
  ThriftProcess();

  /**
   * Closes the process. It will waits until the process exits.
   */
  virtual ~ThriftProcess() noexcept;

public:
  /**
   * Reports the status of the process
   *
   * @return true if the process is running, false if not
   */
  bool isAlive() noexcept;


protected:
  /**
   * Updates _childExitStatus.
   *
   * @param wait_ if true than it waits for process exit.
   */
  void refreshExitStatus(bool wait_);

  /**
   * Does the forking procedure.
   *
   * @param dieWithMe_ if it is true, than the child process will die when the
   *                   parent process (this process) exits.
   * @return 0 if this is the child process, child pid otherwise
   */
  int startProcess(bool dieWithMe_ = true);

  /**
   * Open a socket for initial communication on an available port.
   *
   * @return File descriptor of the created socket.
   */
  int createInitSocket();

  /**
   * Closes a socket.
   */
  void closeSocket(int& socketFd_);

  int getSocketPort(const int& socketFd_);

  /**
   * Waiting and read the port number using child process.
   */
  int waitPort(int& inFd_, const int timeout = 5000);

  /**
    * Creates client according to the given interface and connect to the child
    * process using socket connection.
    */
  template<typename TClient>
  std::unique_ptr<TClient> connect(
    const std::string& host,
    const int timeout = 5000)
  {
    using namespace std::chrono;

    using Transport = apache::thrift::transport::TTransport;
    using ProtocolFactory =
      apache::thrift::protocol::TBinaryProtocolFactoryT<Socket>;

    auto port = waitPort(_initSockedFd, timeout);
    closeSocket(_initSockedFd);
    if (port == -1)
      throw Failure(std::string("Failed to read port."));

    auto socket = boost::make_shared<Socket>(host, port);
    socket->setKeepAlive(true);

    try 
    {
      socket->open();
    }
    catch (std::exception& ex)
    {
      throw Failure(
        std::string("Failed to open socket: ") + host
          + ":" + std::to_string(port) + " - " + ex.what());
    }

    ProtocolFactory protFactory;
    return boost::make_unique<TClient>(protFactory.getProtocol(
      boost::static_pointer_cast<Transport>(socket)));
  }

protected:
  /**
   * Process id of the child process
   */
  int _childPid;

  /**
   * Exit status code of the child process (if any)
   */
  int _childExitStatus;

  /**
   * Socket to read the initial data send by child process.
   */
  int _initSockedFd;
};

} // namespace util
} // namespace cc

#endif // __CODECOMPASS_UTIL_THRIFT_PROCESS_H__
