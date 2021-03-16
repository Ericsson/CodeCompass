#ifndef CC_SERVICE_JAVAPROCESS_H
#define CC_SERVICE_JAVAPROCESS_H

#include <memory>

#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <util/pipedprocess.h>
#include <util/logutil.h>

#include <JavaService.h>

namespace cc
{
namespace service
{
namespace java
{

class JavaProcess : public JavaServiceIf, public util::PipedProcess
{
public:
  class ProcessDied : public apache::thrift::TException
  {
  public:
    ProcessDied() : apache::thrift::TException("Service process died!") {};
  };

  /**
   * Opens the service process.
   *
   * @param indexDatabase_ path to a index database
   */
  JavaProcess(const std::string& indexDatabase_,
              const std::string& compassRoot_) :
  _indexDatabase(indexDatabase_)
  {
    openPipe(_pipeFd2[0], _pipeFd2[1]);

    int pid = startProcess();
    if (pid == 0)
    {
      std::string inFd(std::to_string(_pipeFd[0]));
      std::string outFd(std::to_string(_pipeFd2[1]));

      std::string logLevelOpt("-Dcc.search.logLevel=");
      auto fmtSeverity = util::getSeverityLevel();

      if(fmtSeverity == boost::log::trivial::info)
        logLevelOpt += "INFO";
      else if(fmtSeverity == boost::log::trivial::error ||
              fmtSeverity == boost::log::trivial::warning)
        logLevelOpt += "WARNING";
      else if(fmtSeverity == boost::log::trivial::fatal)
        logLevelOpt += "SEVERE";
      else
        logLevelOpt += "ALL";

      std::string classpath = compassRoot_ + "/lib/java/*";

      ::execlp("java", "java", "-server",
        "-classpath", classpath.c_str(),
        //"-Xdebug", "-Xrunjdwp:transport=dt_socket,server=y,suspend=n,address=8666",
        "-Djava.util.logging.config.class=cc.search.common.config.LogConfigurator",
        "-Djava.util.logging.SimpleFormatter.format=[%4$s] %5$s%6$s%n",
        logLevelOpt.c_str(),
        "cc.java.service.app.service.ServiceApp",
        "-indexDB", _indexDatabase.c_str(),
        "-ipcInFd", inFd.c_str(),
        "-ipcOutFd", outFd.c_str(),
        "-useSimpleFileLock",
        "-cleanupLocks",
        nullptr);

      LOG(error) << "execlp failed!";

      // This shouldn't be executed by child process
      ::abort();
    }
    else
    {
      getClientInterface();
    }
  }

  ~JavaProcess()
  {
    try
    {
      // pleaseStop();
    }
    catch (const ProcessDied&)
    {
    }
    catch (...)
    {
      LOG(warning) << "'pleaseStop' failed!";
    }

    _service.reset(nullptr);

    closePipe(_pipeFd2[0], _pipeFd2[1]);
  }

public:
  void getJavaString(
  std::string& _return) override
  {
    checkProcess();
    _service->getJavaString(_return);
  }

  /*void pleaseStop() override
  {
    checkProcess();
    _service->pleaseStop();
  }*/

private:
  /**
   * Throws a thrift exception if the service process is dead.
   *
   * @throw apache::thrift::TException
   */
  void checkProcess()
  {
    if (!isAlive())
    {
      throw ProcessDied();
    }
  }

  /**
   * Creates the client interface.
   */
  void getClientInterface()
  {
    using Transport = apache::thrift::transport::TFDTransport;
    using ProtocolFactory =
    apache::thrift::protocol::TBinaryProtocolFactoryT<Transport>;

    std::shared_ptr<apache::thrift::transport::TTransport> transIn(
    new Transport(_pipeFd2[0], Transport::NO_CLOSE_ON_DESTROY));
    std::shared_ptr<apache::thrift::transport::TTransport> transOut(
    new Transport(_pipeFd[1], Transport::NO_CLOSE_ON_DESTROY));

    ProtocolFactory protFactory;

    _service.reset(new JavaServiceClient(
    protFactory.getProtocol(transIn),
    protFactory.getProtocol(transOut)));
  }


private:
  /**
   * Path to a index database;
   */
  const std::string _indexDatabase;

  /**
   * Service interface for IPC communication.
   */
  std::unique_ptr<JavaServiceIf> _service;

  /**
   * Second pipe.
   */
  int _pipeFd2[2];
};

} // java
} // service
} // cc

#endif // CC_SERVICE_JAVAPROCESS_H