#include <javaparser-api/javaparserprocess.h>

#include <chrono>
#include <thread>
#include <stdexcept>

#include <boost/make_shared.hpp>
#include <boost/make_unique.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <util/streamlog.h>

namespace cc
{
namespace parser
{

JavaParserProcess::JavaParserProcess()
{
  if (startProcess() == 0)
  {
    std::string logLevelOpt("-Dcc.javaparser.loglevel=");
    switch (util::StreamLog::getLogLevel())
    {
      case util::INFO:
      case util::WARNING:
        logLevelOpt += "INFO";
        break;
      case util::ERROR:
        logLevelOpt += "WARNING";
        break;
      case util::STATUS:
      case util::CRITICAL:
        logLevelOpt += "SEVERE";
        break;
      case util::DEBUG:
      default:
        logLevelOpt += "FINEST";
        break;
    }

    auto port = std::to_string(getSocketPort(_initSockedFd));
    std::vector<const char*> execArguments {
      "java",
      "-Djava.util.logging.config.class=parser.LogConfigurator",
      logLevelOpt.c_str(),
      "parser.JavaParserServiceImpl",
      port.c_str(),
      "2>&1"
    };

    execArguments.push_back(nullptr);
    SLog(util::DEBUG) << "Starting JavaParserProcess...";
    ::execvp("java", const_cast<char* const *>(execArguments.data()));

    SLog(util::ERROR) << "execlp failed!";
    // this shouldn't be executed by child process
    ::abort();
  }
  else
  {
    try
    {
      _javaParser = connect<parser::JavaParserServiceClient>("0.0.0.0");
    }
    catch(std::runtime_error& ex)
    {
      SLog(util::CRITICAL) << "Cannot connect to JavaParser: " << ex.what();
      abort();
    }
  }
}
  
JavaParserProcess::~JavaParserProcess()
{
  if(_javaParser && isAlive())
  {
    _javaParser->stop();
  }

  _javaParser.reset(nullptr);
}
  
JavaParsingResult::type JavaParserProcess::parse(const JavaParserArg& arg_)
{
  return _javaParser->parse(arg_);
}

void JavaParserProcess::stop()
{
  _javaParser->stop();
  _javaParser.reset(nullptr);
}

} // parser
} // cc