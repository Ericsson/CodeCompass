#include <util/pipedprocess.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#if __linux__
#include <sys/prctl.h>
#endif

namespace cc
{
namespace util
{

PipedProcess::Failure::Failure(const std::string& msg_) :
  std::runtime_error(msg_) {}

PipedProcess::PipedProcess() : _pipeFd{0, 0}, _childPid(0), _childExitStatus(0)
{
}

PipedProcess::~PipedProcess() noexcept
{
  closePipe(_pipeFd[0], _pipeFd[1]);

  try
  {
    refreshExitStatus(true);
  }
  catch (...) // Failure&
  {
  }
}

bool PipedProcess::isAlive() noexcept
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

void PipedProcess::refreshExitStatus(bool wait_)
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

int PipedProcess::startProcess(bool dieWithMe_)
{
  if (::pipe(_pipeFd) != 0)
  {
    throw Failure("pipe failed!");
  }

  _childPid = ::fork();
  if (_childPid == -1)
  {
    throw Failure("fork failed!");
  }

#if __linux__
  // prctl.h is not available on macOS
  if (_childPid == 0 && dieWithMe_)
  {
    ::prctl(PR_SET_PDEATHSIG, SIGTERM);
  }
#endif

  return _childPid;
}

void PipedProcess::openPipe(int& inFd_, int& outFd_)
{
  int pipe[2];

  if (::pipe(pipe) != 0)
  {
    throw Failure("pipe failed!");
  }

  inFd_ = pipe[0];
  outFd_ = pipe[1];
}

void PipedProcess::closePipe(int& inFd_, int& outFd_)
{
  if (inFd_ != 0)
  {
    ::close(inFd_);
    inFd_ = 0;
  }

  if (outFd_ != 0)
  {
    ::close(outFd_);
    outFd_ = 0;
  }
}

} // namespace util
} // namespace cc
