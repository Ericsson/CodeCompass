#ifndef CLIENTWRAPPER_H
#define	CLIENTWRAPPER_H

#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/THttpClient.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>

namespace cc
{
namespace util
{

/**
 * RAII class for a Thrift client object.
 * Possible improvement: Reuse clients if performance problems are encountered.
 * Note: Python server does not support reusing yet anyway.
 */
template <typename Client>
class ClientWrapper
{
public:  
  ClientWrapper(const std::string& host_, int port_, const std::string& path_)
  {
    if (!_client)
    {
      using namespace apache::thrift;
      using namespace apache::thrift::protocol;
      using namespace apache::thrift::transport;

      _transport.reset(new THttpClient(host_, port_, path_));
      boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(_transport));
      _client.reset(new Client(protocol));
    }

    // (Re)open transport
    if (!_transport->isOpen())
      _transport->open();
  }
  
  ~ClientWrapper()
  {
    if (_transport->isOpen())
      _transport->close();
  }
  
  boost::shared_ptr<Client> client() const { return _client; }
  
private:
  boost::shared_ptr<apache::thrift::transport::TTransport> _transport;
  boost::shared_ptr<Client> _client;
};

}
}

#endif
