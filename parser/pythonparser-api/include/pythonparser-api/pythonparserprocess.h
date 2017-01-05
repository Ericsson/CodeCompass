#ifndef __CODECOMPASS_PYTHON_PARSER_PROCESS_H__
#define __CODECOMPASS_PYTHON_PARSER_PROCESS_H__

#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <util/pipedprocess.h>
#include <pythonparser-api/PythonPersisterService.h>
#include <parser/traversal.h>

namespace cc
{
namespace parser
{

/**
 * Pyton parser process. Currently it's a Java application.
 */
class PytonhParserProcess : public util::PipedProcess
{
public:
  /**
   * Starts the python parser process.
   *
   * @param path_ a directory / python file.
   */
  PytonhParserProcess(
      const std::vector<std::string> & path_
    , const typename Traversal::OptionMap& om_);

  /**
   * Closes the I/O pipe so the child process will exit if it finished. Also
   * waits for the process to exit.
   */
  ~PytonhParserProcess();

  /**
   * Serves the python java process until it exists.
   *
   * @param pserv_ a persister service implementation.
   */
  void startServe(boost::shared_ptr<PythonPersisterServiceIf> pserv_);

  /**
   * Stops the serving loop.
   */
  void stopServe();

private:
  /**
   * Second pipe for thrift.
   */
  int _pipeFd2[2];
  /**
   * Input protocol for thrift processor.
   */
  boost::shared_ptr<apache::thrift::protocol::TProtocol> _in;
  /**
   * Output protocol for thrift processor.
   */
  boost::shared_ptr<apache::thrift::protocol::TProtocol> _out;
  /**
   * Condition for the serving loop.
   */
  bool _contServe = true;
};

} // parser
} // cc

#endif // __CODECOMPASS_PYTHON_PARSER_PROCESS_H__
