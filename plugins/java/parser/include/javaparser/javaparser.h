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

#include <JavaParserService.h>

#include <chrono>

namespace cc
{
namespace parser
{
namespace java {

namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;
using TransportException = apache::thrift::transport::TTransportException;

class JavaParserServiceHandler : public JavaParserServiceIf {
public:
  JavaParserServiceHandler() {
  }

  void parseFile(long fileId, int fileIndex) override
  {
    _service -> parseFile(fileId, fileIndex);
  }

  void setArgs(const CompileCommand& compileCommand) override
  {
    _service -> setArgs(compileCommand);
  }

  void getArgs(CmdArgs& _return) override
  {
    _service -> getArgs(_return);
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

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    while (!transport->isOpen()) {
      try {
        transport->open();
      } catch (TransportException& ex) {
        chrono::steady_clock::time_point current = chrono::steady_clock::now();
        float elapsed_time =
          chrono::duration_cast<chrono::milliseconds>(current - begin).count();

        if (elapsed_time > timeout_in_ms) {
          LOG(debug) << "Connection refused, could not reach Java server on"
            << host << ":" << port;
          throw ex;
        }
      }
    }

    _service.reset(new JavaParserServiceClient(protocol));
  }

private:
  /**
   * Service interface for IPC communication.
   */
  std::unique_ptr<JavaParserServiceIf> _service;
};

class JavaParser : public AbstractParser {
public:
  JavaParser(ParserContext &ctx_);

  virtual ~JavaParser();

  virtual bool parse() override;

private:
  fs::path _java_path;

  bool accept(const std::string &path_);

  CompileCommand getCompileCommand(
    const pt::ptree::value_type &command_tree_);

  model::BuildActionPtr addBuildAction(
    const CompileCommand &compile_command_);

  void addCompileCommand(
    const CmdArgs &cmd_args_,
    model::BuildActionPtr buildAction_,
    short parse_state_);
};

} // java
} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
