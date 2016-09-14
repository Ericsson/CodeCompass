#include <memory>
#include <iostream>

#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <uscs-api/uscs_constants.h>

#include "uscshandler.h"
#include "model/database.h"

// undef thrift config.h`s stuff
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include <config.h>

using namespace cc::service::stat;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

namespace
{

/**
 * Helper struct to keep options and the db object.
 */
struct Options
{
  /**
   * Server port.
   */
  int port;
  /**
   * The user provided connection string.
   */
  std::string connectionString;
  /**
   * The database instance.
   */
  std::shared_ptr<odb::database> database;
};

/**
 * Event handler for setting fallback client address.
 */
class ConnEventHandler : public TServerEventHandler
{
public:
  virtual void processContext(void*,
    boost::shared_ptr<TTransport> trans_) override
  {
    try
    {
      auto sock = dynamic_cast<TSocket*>(trans_.get());
      if (sock)
      {
        UsageStatisticsServiceHandler::setCurrentClientAddress(
          sock->getPeerHost());
      }
    }
    catch (...)
    {
    }
  }
};

/**
 * Creates a thrift server on the given port (see Options).
 *
 * @param opt_ server options + db
 * @return a server instance
 */
std::unique_ptr<TServer> createServer(Options opt_)
{
  // Use UsageStatisticsServiceHandler for handler
  auto procFact = boost::make_shared<UsageStatisticsServiceProcessorFactory>(
    boost::make_shared<UsageStatisticsServiceIfSingletonFactory>(
      boost::make_shared<UsageStatisticsServiceHandler>(opt_.database)));
  // Use a server socket on the given port
  auto serverTrans = boost::make_shared<TServerSocket>(opt_.port);
  // Use binary protocol
  auto protFact = boost::make_shared<TBinaryProtocolFactory>();
  // Use the default transport factory
  auto transFact = boost::make_shared<TTransportFactory>();

  auto srv = std::unique_ptr<TServer>(new TThreadedServer(
    procFact, serverTrans, transFact, protFact));
  srv->setServerEventHandler(boost::make_shared<ConnEventHandler>());

  return srv;
}

/**
 * Prints the program usage.
 *
 * @param prog_ program's name
 * @param desc_ options description
 */
void printUsage(
  const char* prog_,
  const boost::program_options::options_description& desc_)
{
  std::cerr << "USAGE: " << prog_ << " " << desc_ << std::endl;
}

/**
 * Processes the command line.
 *
 * @param argc_ arg cout from main function
 * @param argv_ arg vector from main function
 * @return the program options
 */
Options processCommandLine(int argc_, const char* argv_[])
{
  namespace po = boost::program_options;

  Options opt;
  po::options_description options("options");
  options.add_options()
    ("port,p",
      po::value<int>(&opt.port)->
#ifndef CC_USCS_DEFAULT_PORT
        required(),
#else
        default_value(CC_USCS_DEFAULT_PORT),
#endif
      "Server port")
    ("connection,c",
      po::value<std::string>(&opt.connectionString)->required(),
      "Database connection string")
    ("help,h", "Produce help message");

  try
  {
    po::variables_map vm;
    po::store(po::parse_command_line(argc_, argv_, options), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      printUsage(argv_[0], options);
      std::exit(0);
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "ERROR: " << ex.what() << std::endl;
    
    printUsage(argv_[0], options);
    std::exit(-1);
  }

  return opt;
}

} // anonymous namespace

int main(int argc_, const char* argv_[])
{
  Options opt = processCommandLine(argc_, argv_);

  // Open database connection
  opt.database = openDatabase(opt.connectionString);
  if (!opt.database)
  {
    std::cerr << "Failed to open database!" << std::endl;
    return -1;
  }

  auto server = createServer(opt);
  std::cout << "Serving on port " << opt.port << std::endl;
  server->serve();
  std::cout << "Exiting..." << std::endl;

  return 0;
}

