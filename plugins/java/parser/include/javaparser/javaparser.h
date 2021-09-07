#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <model/buildaction.h>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <JavaParserService.h>

namespace cc
{
namespace parser
{
namespace java {

namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;

class JavaParserServiceHandler : public JavaParserServiceIf {
public:
  JavaParserServiceHandler() {
    getClientInterface();
  }

  void parseFile(const CompileCommand& compile_command) override
  {
    _service -> parseFile(compile_command);
  }

private:
  /**
 * Creates the client interface.
 */
  void getClientInterface()
  {
    using Transport = apache::thrift::transport::TTransport;
    using BufferedTransport = apache::thrift::transport::TBufferedTransport;
    using Socket = apache::thrift::transport::TSocket;
    using Protocol = apache::thrift::protocol::TBinaryProtocol;

    std::shared_ptr<Transport>
      socket(new Socket("localhost", 9090));
    std::shared_ptr<Transport>
      transport(new BufferedTransport(socket));
    std::shared_ptr<Protocol>
      protocol(new Protocol(transport));

    transport -> open();
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

  model::BuildActionPtr addBuildAction(
    const pt::ptree::value_type &command_);

  void addCompileCommand(
    const pt::ptree::value_type &command_,
    model::BuildActionPtr buildAction_,
    bool error_ = false);
};

} // java
} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
