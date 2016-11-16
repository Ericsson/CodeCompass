#ifndef CC_UTIL_PIPEDPROCESS_H
#define CC_UTIL_PIPEDPROCESS_H

#include <cstdint>
#include <string>
#include <stdexcept>

namespace cc
{
namespace util
{

/**
 * Helper class for starting a process with a pipe.
 */
class PipedProcess
{
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
  PipedProcess(const PipedProcess&) = delete;
  PipedProcess& operator=(const PipedProcess&) = delete;

  /**
   * Inits the members.
   */
  PipedProcess();

  /**
   * Closes the process. It will wait until the process exits.
   */
  virtual ~PipedProcess() noexcept;

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
   * @param wait_ If true then it waits for process exit.
   */
  void refreshExitStatus(bool wait_);

  /**
   * Does the forking procedure.
   *
   * @param dieWithMe_ If it is true, then the child process will die when the
   *   parent process (this process) exits.
   * @return 0 If this is the child process, child PID otherwise.
   */
  int startProcess(bool dieWithMe_ = true);

  /**
   * Opens a pipe.
   *
   * @param inFd_ Reader end.
   * @param outFd_ Writer end.
   */
  void openPipe(int& inFd_, int& outFd_);

  /**
   * Closes a pipe.
   *
   * @param inFd_ Reader end.
   * @param outFd_ Writer end.
   */
  void closePipe(int& inFd_, int& outFd_);

protected:
  /**
   * Pipe file descriptors. The first is the input FD, the second is output.
   */
  int _pipeFd[2];
  /**
   * Process id of the child process.
   */
  int _childPid;
  /**
   * Exit status code of the child process (if any).
   */
  int _childExitStatus;
};

} // namespace util
} // namespace cc

#endif // CC_UTIL_PIPEDPROCESS_H
