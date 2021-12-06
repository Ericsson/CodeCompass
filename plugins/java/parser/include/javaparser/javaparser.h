#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <model/buildaction.h>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <ProjectService.h>
#include <JavaParserService.h>

#include <iostream>
#include <chrono>

namespace cc
{
namespace parser
{
namespace java
{

namespace core = cc::service::core;
namespace ba = boost::algorithm;
namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;
namespace chrono = std::chrono;
using TransportException = apache::thrift::transport::TTransportException;

class TimeoutException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class JavaParserServiceHandler : public JavaParserServiceIf {
public:
  JavaParserServiceHandler() {
  }

  void parseFile(
    ParseResult& return_,
    const CompileCommand& compileCommand_, long fileId_,
    const std::string& fileCounterStr_) override
  {
    _service -> parseFile(return_, compileCommand_, fileId_, fileCounterStr_);
  }

  void decompileClass(
    std::string& return_, const std::string& path_) override {
    _service -> decompileClass(return_, path_);
  }

  /**
   * Creates the client interface.
   */
  void getClientInterface(int timeout_in_ms, int worker_num)
  {
    using Transport = apache::thrift::transport::TTransport;
    using BufferedTransport = apache::thrift::transport::TBufferedTransport;
    using Socket = apache::thrift::transport::TSocket;
    using Protocol = apache::thrift::protocol::TBinaryProtocol;

    std::string host = "localhost";
    int port = 9090;

    std::shared_ptr<Transport>
      socket(new Socket(host, port));
    std::shared_ptr<Transport>
      transport(new BufferedTransport(socket));
    std::shared_ptr<Protocol>
      protocol(new Protocol(transport));

    // Redirect Thrift output into std::stringstream
    apache::thrift::GlobalOutput.setOutputFunction(
      [](const char* x) {thrift_ss << x;});

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    while (!transport->isOpen()) {
      try {
        transport->open();

        if (!server_started) {
          LOG(info) << "[javaparser] Java server started!";
          server_started = true;
        }

        LOG(info) << "[javaparser] Worker (" <<
          worker_num << ") connected to the Java server!";
      } catch (TransportException& ex) {
        chrono::steady_clock::time_point current = chrono::steady_clock::now();
        float elapsed_time =
          chrono::duration_cast<chrono::milliseconds>(current - begin).count();

        if (elapsed_time > timeout_in_ms) {
          LOG(error) << "Connection timeout, could not reach Java server on"
            << host << ":" << port;
          apache::thrift::GlobalOutput.setOutputFunction(
            apache::thrift::TOutput::errorTimeWrapper);
          throw ex;
        }
      }
    }

    apache::thrift::GlobalOutput.setOutputFunction(
      apache::thrift::TOutput::errorTimeWrapper);

    _service.reset(new JavaParserServiceClient(protocol));
  }

  void reserve() {
    reserved = true;
  }

  void setFree() {
    reserved = false;
  }

  bool isFree() {
    return !reserved;
  }


private:
  /**
   * Service interface for IPC communication.
   */
  std::unique_ptr<JavaParserServiceIf> _service;

  /**
    * Handler's state
    */
  bool reserved = false;

  /**
   * Server's state.
   */
  static bool server_started;

  /**
   * Object to store Thrift messages during connecting to the Java server
   */
  static std::stringstream thrift_ss;
};

class JavaParser : public AbstractParser {
public:
  JavaParser(ParserContext& ctx_);

  virtual ~JavaParser();

  virtual bool parse() override;

private:
  /**
   * A single build command's cc::util::JobQueueThreadPool job.
   */
  struct ParseJob
  {
    /**
     * The build command itself.
     * This is given to a free worker in JavaParser::_javaServiceHandlers.
     */
    const CompileCommand command;

    /**
     * The # of the build command in the compilation command database.
     */
    std::size_t index;

    ParseJob(const CompileCommand command, std::size_t index)
      : command(command), index(index)
    {}

    ParseJob(const ParseJob&) = default;
  };

  /**
 * A single Java class file's cc::util::JobQueueThreadPool job.
 */
  struct DecompileJob
  {
    /**
     * The path to the Java class file.
     * This is given to a free worker in JavaParser::_javaServiceHandlers.
     */
    const std::string path;

    DecompileJob(const std::string& path) : path(path)
    {}

    DecompileJob(const DecompileJob&) = default;
  };

  fs::path _java_path;
  fs::path _unzip_path;
  pr::child _c;
  std::unique_ptr<util::JobQueueThreadPool<ParseJob>> _parsePool;
  int _numCompileCommands;
  int _threadNum;
  std::vector<std::shared_ptr<JavaParserServiceHandler>> _javaServiceHandlers;

  bool acceptCompileCommands(const std::string& path_);

  bool acceptJar(const std::string& path_);

  bool acceptClass(const std::string& path_);

  bool rejectInnerClass(const std::string& path_);

  void startAndConnectToJavaProcess();

  void initializeWorkers();

  std::shared_ptr<JavaParserServiceHandler>& findFreeWorker(int timeout_in_ms_);

  CompileCommand getCompileCommandFromJson(
    const pt::ptree::value_type& command_tree_);

  CompileCommand getCompileCommandForDecompiledFile(
    const std::string& filePath_, const std::string& classpath_,
    const std::string& sourcepath_);

  std::string getClasspathFromMetaInf(const fs::path& root);

  model::BuildActionPtr addBuildAction(const CompileCommand& compile_command_);

  void addCompileCommand(
    const CmdArgs& cmd_args_,
    model::BuildActionPtr buildAction_,
    model::File::ParseStatus parseStatus_);

  model::File::ParseStatus addBuildLogs(
    const std::vector<core::BuildLog>& buildLogs_,
    const std::string& file_);

  bool parseCompileCommands(const std::string& path_);

  bool parseJar(const std::string& path_);

  std::vector<CompileCommand> decompileJar(const std::string& path_);
};

} // java
} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
