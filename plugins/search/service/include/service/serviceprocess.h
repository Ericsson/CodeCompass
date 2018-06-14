#ifndef CC_SERVICE_SERVICEPROCESS_H
#define CC_SERVICE_SERVICEPROCESS_H

#include <memory>

#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <util/pipedprocess.h>
#include <util/logutil.h>

#include <SearchService.h>

namespace cc
{
namespace service
{
namespace search
{

class ServiceProcess : public SearchServiceIf, public util::PipedProcess
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
  ServiceProcess(const std::string& indexDatabase_,
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
        "cc.search.service.app.service.ServiceApp",
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

  ~ServiceProcess()
  {
    try
    {
      pleaseStop();
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
  void search(
    SearchResult& _return,
    const SearchParams& params_) override
  {
    checkProcess();
    _service->search(_return, params_);
  }

  void searchFile(
    FileSearchResult& _return,
    const SearchParams& params_) override
  {
    checkProcess();
    _service->searchFile(_return, params_);
  }

  void getSearchTypes(
    std::vector<SearchType>& _return) override
  {
    checkProcess();
    _service->getSearchTypes(_return);
  }

  void pleaseStop() override
  {
    checkProcess();
    _service->pleaseStop();
  }

  void suggest(SearchSuggestions& _return,
    const SearchSuggestionParams& params_) override
  {
    checkProcess();
    _service->suggest(_return, params_);
  }

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

    boost::shared_ptr<apache::thrift::transport::TTransport> transIn(
      new Transport(_pipeFd2[0], Transport::NO_CLOSE_ON_DESTROY));
    boost::shared_ptr<apache::thrift::transport::TTransport> transOut(
      new Transport(_pipeFd[1], Transport::NO_CLOSE_ON_DESTROY));

    ProtocolFactory protFactory;

    _service.reset(new SearchServiceClient(
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
  std::unique_ptr<SearchServiceIf> _service;

  /**
   * Second pipe.
   */
  int _pipeFd2[2];
};

} // search
} // service
} // cc

#endif // CC_SERVICE_SERVICEPROCESS_H

