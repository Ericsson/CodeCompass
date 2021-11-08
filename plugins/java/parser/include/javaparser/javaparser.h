#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

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
namespace java {

namespace core = cc::service::core;
namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;
using TransportException = apache::thrift::transport::TTransportException;

class JavaParserServiceHandler : public JavaParserServiceIf {
public:
  JavaParserServiceHandler() {
  }

  void parseFile(
    std::vector<core::BuildLog>& return_, long fileId_, int fileIndex_) override
  {
    _service -> parseFile(return_, fileId_, fileIndex_);
  }

  void setArgs(const CompileCommand& compileCommand_) override
  {
    _service -> setArgs(compileCommand_);
  }

  void getArgs(CmdArgs& return_) override
  {
    _service -> getArgs(return_);
  }

  /**
   * Creates the client interface.
   */
  void getClientInterface(int timeout_in_ms)
  {
    using Transport = apache::thrift::transport::TTransport;
    using BufferedTransport = apache::thrift::transport::TBufferedTransport;
    using Socket = apache::thrift::transport::TSocket;
    using Protocol = apache::thrift::protocol::TBinaryProtocol;
    namespace chrono = std::chrono;

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
      } catch (TransportException& ex) {
        chrono::steady_clock::time_point current = chrono::steady_clock::now();
        float elapsed_time =
          chrono::duration_cast<chrono::milliseconds>(current - begin).count();

        if (elapsed_time > timeout_in_ms) {
          LOG(debug) << "Connection timeout, could not reach Java server on"
            << host << ":" << port;
          apache::thrift::GlobalOutput.setOutputFunction(
            apache::thrift::TOutput::errorTimeWrapper);
          throw ex;
        }
      }
    }

    apache::thrift::GlobalOutput.setOutputFunction(
      apache::thrift::TOutput::errorTimeWrapper);

    LOG(info) << "[javaparser] Java server started!";

    _service.reset(new JavaParserServiceClient(protocol));
  }

private:
  /**
   * Service interface for IPC communication.
   */
  std::unique_ptr<JavaParserServiceIf> _service;

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
  fs::path _java_path;

  bool accept(const std::string& path_);

  CompileCommand getCompileCommand(
    const pt::ptree::value_type& command_tree_);

  model::BuildActionPtr addBuildAction(
    const CompileCommand& compile_command_);

  void addCompileCommand(
    const CmdArgs& cmd_args_,
    model::BuildActionPtr buildAction_,
    model::File::ParseStatus parseStatus_);

  model::File::ParseStatus addBuildLogs(
    const std::vector<core::BuildLog>& buildLogs_,
    std::string& file_);
};

} // java
} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
