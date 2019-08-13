#include <sstream>
#include <memory>

#include <boost/log/expressions.hpp>

#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <util/logutil.h>

#include <indexer/indexerprocess.h>

#ifdef JAVAMEMORYAMOUNT
#undef JAVAMEMORYAMOUNT
#endif

#ifdef __i386
  // JVM won't start with -Xmx2g on 32-bit platforms
  #define JAVAMEMORYAMOUNT "-Xmx1532m"
#else
  #define JAVAMEMORYAMOUNT "-Xmx2g"
#endif

namespace cc
{
namespace parser
{

IndexerProcess::IndexerProcess(
  const std::string& indexDatabase_,
  const std::string& compassRoot_,
  IndexerProcess::OpenMode openMode_,
  IndexerProcess::LockMode lockMode_)
{
  openPipe(_pipeFd2[0], _pipeFd2[1]);

  if (startProcess() == 0)
  {
    // This is the child process.
    std::string inFd(std::to_string(_pipeFd[0]));
    std::string outFd(std::to_string(_pipeFd2[1]));

    std::string logLevelOpt("-Dcc.search.logLevel=");

    auto fmtSeverity = util::getSeverityLevel();

    if (fmtSeverity == boost::log::trivial::info)
      logLevelOpt += "INFO";
    else if (fmtSeverity == boost::log::trivial::error ||
             fmtSeverity == boost::log::trivial::warning)
      logLevelOpt += "WARNING";
    else if (fmtSeverity == boost::log::trivial::fatal)
      logLevelOpt += "SEVERE";
    else
      logLevelOpt += "FINEST";

    std::string classpath = compassRoot_ + "/lib/java/*";

    std::vector<const char*> execArguments {
      "java", JAVAMEMORYAMOUNT,
      "-classpath", classpath.c_str(),
      "-Djava.util.logging.config.class=cc.search.common.config.LogConfigurator",
      "-Djava.util.logging.SimpleFormatter.format=[%4$s] %5$s%6$s%n",
      logLevelOpt.c_str(),
      "cc.search.indexer.app.Indexer",
      "-indexDB", indexDatabase_.c_str(),
      "-ipcInFd", inFd.c_str(),
      "-ipcOutFd", outFd.c_str()
    };

    switch (openMode_)
    {
      case OpenMode::Create:
        execArguments.push_back("-create");
        break;
      case OpenMode::ReplaceExisting:
        execArguments.push_back("-replaceExisting");
        break;
      case OpenMode::Merge:
        execArguments.push_back("-merge");
        break;
    }

    switch (lockMode_)
    {
      case LockMode::Native:
        // This is the default
        break;
      case LockMode::Simple:
        execArguments.push_back("-useSimpleFileLock");
        execArguments.push_back("-cleanupLocks");
        break;
    }

    execArguments.push_back(nullptr);
    ::execvp("java", const_cast<char* const*>(execArguments.data()));

    LOG(error) << "execlp failed!";
    // this shouldn't be executed by child process
    ::abort();
  }
  else
  {
    // Get the client interface
    using Transport = apache::thrift::transport::TFDTransport;
    using ProtocolFactory =
      apache::thrift::protocol::TBinaryProtocolFactoryT<Transport>;

    std::shared_ptr<apache::thrift::transport::TTransport> transIn(
      new Transport(_pipeFd2[0], Transport::NO_CLOSE_ON_DESTROY));
    std::shared_ptr<apache::thrift::transport::TTransport> transOut(
      new Transport(_pipeFd[1], Transport::NO_CLOSE_ON_DESTROY));

    ProtocolFactory protFactory;

    _indexer.reset(new search::IndexerServiceClient(
      protFactory.getProtocol(transIn),
      protFactory.getProtocol(transOut)));
  }
}

IndexerProcess::~IndexerProcess()
{
  if (_indexer && isAlive())
    _indexer->stop();

  closePipe(_pipeFd[0], _pipeFd[1]);
  closePipe(_pipeFd2[0], _pipeFd2[1]);

  _indexer.reset(nullptr);
}

void IndexerProcess::stop()
{
  assert(false && "IndexerProcess::stop called");
}

void IndexerProcess::indexFile(
  const std::string& fileId_,
  const std::string& filePath_,
  const std::string& mimeType_)
{
  if (!isAlive())
  {
    LOG(error) << "Index process is not alive!";
    ::abort();
  }
  
  _indexer->indexFile(fileId_, filePath_, mimeType_);
}
  
void IndexerProcess::addFieldValues(
  const std::string& fileId_,
  const search::Fields& fields_)
{
  if (!isAlive())
  {
    LOG(error) << "Index process is not alive!";
    ::abort();
  }

  _indexer->addFieldValues(fileId_, fields_);
}

void IndexerProcess::buildSuggestions()
{
  if (!isAlive())
  {
    LOG(error) << "Index process is not alive!";
    ::abort();
  }

  _indexer->buildSuggestions();
}

void IndexerProcess::getStatistics(std::map<std::string, std::string>& stat_)
{
  if (!isAlive())
  {
    LOG(error) << "Index process is not alive!";
    ::abort();
  }

  _indexer->getStatistics(stat_);
}

} // namespace parser
} // namespace cc
