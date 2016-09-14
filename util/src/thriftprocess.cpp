#include <util/thriftprocess.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <climits>
#include <errno.h>
#include <string.h>

#include <util/streamlog.h>

namespace cc
{
namespace util
{

ThriftProcess::Failure::Failure(const std::string& msg_) :
  std::runtime_error(msg_) {}

ThriftProcess::ThriftProcess() :
  _childPid(0),
  _childExitStatus(0),
  _initSockedFd(0)
{
}

ThriftProcess::~ThriftProcess() noexcept
{
  closeSocket(_initSockedFd);

  try
  {
    refreshExitStatus(true);
  }
  catch (...) // Failure&
  {
  }
}

bool ThriftProcess::isAlive() noexcept
{
  try
  {
    refreshExitStatus(false);
    return _childPid != 0;
  }
  catch (...) // Failure&
  {
    return false;
  }
}

void ThriftProcess::refreshExitStatus(bool wait_)
{
  if (_childPid <= 0)
  {
    return;
  }

  pid_t res = ::waitpid(_childPid, &_childExitStatus, wait_ ? 0 : WNOHANG);
  if (res < 0)
  {
    _childPid = 0;
    throw Failure("waitpid failed!");
  }
  else if (res > 0)
  {
    _childPid = 0;
  }
}

int ThriftProcess::startProcess(bool dieWithMe_)
{
  _initSockedFd = createInitSocket();

  _childPid = ::fork();
  if (_childPid == -1)
  {
    throw Failure("fork failed!");
  }

  if (_childPid == 0 && dieWithMe_)
  {
    ::prctl(PR_SET_PDEATHSIG, SIGTERM);
  }

  return _childPid;
}

void ThriftProcess::closeSocket(int& socketFd_)
{
  if (socketFd_ == 0)
    return;

  close(socketFd_);
  socketFd_ = 0;
}

int ThriftProcess::getSocketPort(const int& socketFd_)
{
  struct sockaddr_in server_addr;
  socklen_t saddr_len = sizeof(server_addr);
  
  if (getsockname(socketFd_, (struct sockaddr *)(&server_addr), &saddr_len) == 0)
  {
    return ntohs(server_addr.sin_port);
  }
  else
  {
    throw Failure(
      std::string("Some error occurred while getting obtained port number: ")
      + strerror(errno));
  }
}

int ThriftProcess::createInitSocket()
{
  int socketFd;
  if ((socketFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    throw Failure(
      std::string("Socket can not be constructed: ") + strerror(errno));
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(0);

  if (bind(socketFd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) != 0)
  {
    throw Failure(
      std::string("Cannot bind to socket: ") + strerror(errno));
  }

  listen(socketFd, 1);

  return socketFd;
}

int ThriftProcess::waitPort(int& inFd_, const int timeout)
{
  if (inFd_ == 0)
    return -1;
  
  struct sockaddr_in client_addr;
  socklen_t caddr_len = sizeof(client_addr);
  
  int socketfd;
  if ((socketfd = accept(inFd_, (struct sockaddr *) &client_addr, &caddr_len)) < 0)
  {
    SLog(util::DEBUG) << "Failed at accept: " << strerror(errno);
    return -1;
  }

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(socketfd, &readfds);

  struct timeval tv;
  tv.tv_sec  = timeout / 1000;
  tv.tv_usec = timeout % 1000;

  int s;
  if ((s = select(socketfd + 1, &readfds, nullptr, nullptr, &tv)) < 0)
  {
    SLog(util::DEBUG)
      << "Failed at select: " << (s == 0) ? "Timeout." : strerror(errno);
    return -1;
  }

  std::int32_t port = 0;
  if (recv(socketfd, &port, sizeof(port), MSG_WAITALL) == -1)
    return -1;

  return ntohl(port);
}

} // namespace util
} // namespace cc
