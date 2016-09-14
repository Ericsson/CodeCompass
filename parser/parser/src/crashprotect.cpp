#include <parser/crashprotect.h>

#include <util/streamlog.h>

#include <iostream>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

namespace
{

/**
 * A (smart) pointer to the current action for the SIGSEGV handler. Its thread
 * local so we don't need a complex algorithm in the signal handler to detect
 * which action caused the error.
 *
 * A quote from the book "The Linux Programming Interface":
 * A signal may be directed to either the process as a whole or to a specific
 * thread. A signal is thread-directed if:
 *   - it is generated as the direct result of the execution of a specific
 *     hardware instruction within the context of the thread (i.e., the hardware
 *     exceptions ... : SIGBUS, SIGFPE, SIGILL, and SIGSEGV)
 *   ...
 */
thread_local cc::model::BuildActionPtr currentAction;

/**
 * Context string for the current action.
 */
thread_local std::string currentActionContext;

/**
 * Signal handler for SIGSEGV and SIGABRT signal. This function just prints some
 * text to standard output for the parser script (eg.: runparse.sh).
 *
 * SIGABRT is usually sent by a raise() call, so it will handled in the calling
 * thread.
 *
 * At the end of the function it propagates the signal to the default handler.
 *
 * For parameters see the POSIX/LINUX documentation.
 *
 * The output can be:
 *    CODECOMPASS CRASH ON ACTION $actionid
 * or CODECOMPASS CRASH ON UNKNOWN ACTION
 */
void errorHandler(int sig_, siginfo_t* siginfo_, void* context_)
{
  if (sig_ == SIGSEGV || sig_ == SIGABRT)
  {
    std::cout << std::endl << std::endl << "CODECOMPASS CRASH ON ";
    if (!currentAction)
    {
      std::cout << "UNKNOWN ACTION";
    }
    else
    {
      std::cout << "ACTION " << currentAction->id;
    }
    std::cout << std::endl;

    if (!currentActionContext.empty())
    {
      std::cout
       << "*** CRASH DIAGNOSTICS ***" << std::endl
       << currentActionContext << std::endl
       << "*** END CRASH DIAGNOSTICS ***" << std::endl
       << std::endl;
    }
  }
  else
  {
    SLog(cc::util::WARNING) << "Wrong signal: " << sig_ << "!";
  }

  {
    std::ostringstream ofilename;
    ofilename << "dummy.core";

    if (currentAction)
    {
      ofilename << "." << currentAction->id;
    }

    std::ofstream crash(ofilename.str());
    crash << "A crash has happened but no real core dump was created." << std::endl;
    crash.flush();
    crash.close();
  }

  // Propagate this signal to the default signal handler to get a nice core
  // file.
  ::signal(sig_, SIG_DFL);
  ::kill(::getpid(), sig_);
}

} // anonymous namespace

namespace cc
{
namespace parser
{
namespace crash
{

void initHandler()
{
  struct sigaction act;
  ::memset(&act, 0, sizeof(act));
  ::sigemptyset(&act.sa_mask);

  act.sa_sigaction = &errorHandler;
  act.sa_flags = SA_SIGINFO;

  if (::sigaction(SIGSEGV, &act, nullptr) != 0)
  {
    SLog(util::WARNING) << "Failed to set SIGSEGV handler!";
  }

  if (::sigaction(SIGABRT, &act, nullptr) != 0)
  {
    SLog(util::WARNING) << "Failed to set SIGABRT handler!";
  }
}

void setAction(model::BuildActionPtr action_, std::string contextText_)
{
  currentAction = action_;
  currentActionContext = std::move(contextText_);
}

} // crash
} // parser
} // cc
