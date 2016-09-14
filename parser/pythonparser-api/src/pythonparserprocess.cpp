#include <pythonparser-api/pythonparserprocess.h>

#include <sstream>

#include <util/util.h>
#include <util/streamlog.h>

#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

namespace cc
{
namespace parser
{

PytonhParserProcess::PytonhParserProcess(
    const std::vector<std::string> & paths_
  , const typename Traversal::OptionMap& om_)
{
  openPipe(_pipeFd2[0], _pipeFd2[1]);

  if (startProcess() == 0)
  {
    // This is the child process
    std::string inFd(std::to_string(_pipeFd[0]));
    std::string outFd(std::to_string(_pipeFd2[1]));

    std::string joinedPath = "";
    for(auto & path : paths_)
    {
      joinedPath += path + " ";
    }

    std::string logLevel;

    switch (util::StreamLog::getLogLevel())
    {
      case util::INFO:
      case util::WARNING:
        logLevel = "INFO";
        break;
      case util::ERROR:
        logLevel = "WARNING";
        break;
      case util::STATUS:
      case util::CRITICAL:
        logLevel = "SEVERE";
        break;
      case util::DEBUG:
      default:
        logLevel = "FINEST";
        break;
    }
    std::string memLimit =  std::string("-Xmx") 
                          + om_.at("python-mem-limit")
                          + std::string("m");
    std::string depsPath = om_.at("python-deps");

    


#ifdef __i386
    //JVM won't start with -Xmx2g on 32-bit platforms
    #define PLATFORM "32bit"
#else
    #define PLATFORM "64bit"
#endif

    if(strcmp(PLATFORM, "32bit") == 0)
    {
      memLimit = "-Xmx1532m";
    }

    // // It's basically for unit testing on a low-memory jenkins build server but
    // // it could be useful on other computers too.
    // std::string memLimit(PYTHONPARSER_JAVAMEMORYAMOUNT);
    // const char* envMemLimit = std::getenv("CC_PYTHONPARSER_JAVAMEMORYAMOUNT");
    // if (envMemLimit)
    // {
    //   memLimit  = "-Xmx";
    //   memLimit += envMemLimit;
    // }

    // SLog(util::ERROR) << "\n\n\n" << "MEMORY: " << memLimit << "\n\n\n";
    // SLog(util::ERROR) << "\n\n\n" << "Python deps: " << depsPath << "\n\n\n";

    ::execlp("java", "java", 
      memLimit.c_str(),
      "org.yinwang.pysonar.PythonParser",
      //"-deps", "/usr/lib/python2.7/",
      "-deps", depsPath.c_str(),
      "-log", logLevel.c_str(),
      //"-quiet",
      joinedPath.c_str(),
      "-ipcInFd", inFd.c_str(),
      "-ipcOutFd", outFd.c_str(),

      nullptr);

    SLog(util::ERROR) << "execlp failed!";
    // this shouldn't be executed by child process
    ::abort();
  }
  else
  {
    // Get the client interface
    using Transport = apache::thrift::transport::TFDTransport;
    using ProtocolFactory =
      apache::thrift::protocol::TBinaryProtocolFactoryT<Transport>;

    boost::shared_ptr<apache::thrift::transport::TTransport> transIn(
      new Transport(_pipeFd2[0], Transport::NO_CLOSE_ON_DESTROY));
    boost::shared_ptr<apache::thrift::transport::TTransport> transOut(
      new Transport(_pipeFd[1], Transport::NO_CLOSE_ON_DESTROY));

    ProtocolFactory protFactory;
    _in = protFactory.getProtocol(transIn);
    _out = protFactory.getProtocol(transOut);
  }
}

PytonhParserProcess::~PytonhParserProcess()
{
  closePipe(_pipeFd[0], _pipeFd[1]);
  closePipe(_pipeFd2[0], _pipeFd2[1]);
}

void PytonhParserProcess::startServe(
  boost::shared_ptr<PythonPersisterServiceIf> pserv_)
{
  PythonPersisterServiceProcessor proc(pserv_);

  _contServe = true;
  while (isAlive() && _contServe && proc.process(_in, _out, nullptr));
}

void PytonhParserProcess::stopServe()
{
  _contServe = false;
}

} // namespace parser
} // namespace cc

