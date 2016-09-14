#ifndef __CODECOMPASS_UTIL_PIPED_PROCESS_H__
#define __CODECOMPASS_UTIL_PIPED_PROCESS_H__

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
  PipedProcess& operator = (const PipedProcess&) = delete;

  /**
   * Inits the members.
   */
  PipedProcess();

  /**
   * Closes the process. It will waits until the process exits.
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
   * Opens a pipe.
   *
   * @param inFd_ reader end
   * @param outFd_ writer end
   */
  void openPipe(int& inFd_, int& outFd_);

  /**
   * Closes a pipe.
   *
   * @param inFd_ reader end
   * @param outFd_ writer end
   */
  void closePipe(int& inFd_, int& outFd_);

protected:
  /**
   * Pipe file descriptors. The first is the input fd, the second is output.
   */
  int _pipeFd[2];
  /**
   * Process id of the child process
   */
  int _childPid;
  /**
   * Exit status code of the child process (if any)
   */
  int _childExitStatus;
};

} // namespace util
} // namespace cc

#endif // __CODECOMPASS_UTIL_PIPED_PROCESS_H__
