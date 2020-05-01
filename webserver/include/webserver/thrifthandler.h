#ifndef CC_WEBSERVER_THRIFTHANDLER_H
#define CC_WEBSERVER_THRIFTHANDLER_H

#include <cstdio>
#include <memory>

#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/THttpServer.h>
#include <thrift/transport/TTransport.h>
#include <thrift/protocol/TJSONProtocol.h>

#include <util/logutil.h>

#include "requesthandler.h"

/**
 * Returns the demangled name of the type described by the given type info.
 *
 * @param info_ a type info.
 * @return the pretty name of the type.
 */
inline std::string getTypeName(const std::type_info& info_)
{
  int status = 0;
  std::unique_ptr<char[], void (*)(void*)> result(
    abi::__cxa_demangle(info_.name(), 0, 0, &status), std::free);

  return result.get() ? std::string(result.get()) : "##error##";
}

/**
 * Returns the template argument's demangled name.
 *
 * @return the pretty name of the T type.
 */
template <typename T>
inline std::string getTypeName()
{
  return getTypeName(typeid(T));
}

namespace cc
{
namespace webserver
{

template<class Processor>
class ThriftHandler : public RequestHandler
{
protected:
  /**
   * Calling context for thrift process calls.
   */
  struct CallContext
  {
    /**
     * HTTP request received.
     */
    const HTTPRequest* request;

    /**
     * A pointer for the real call context (for dispatch call).
     */
    void* nextCtx;
  };

  class LoggingProcessor : public Processor
  {
  public:
    template <typename IFaceType>
    LoggingProcessor(std::shared_ptr<IFaceType> handler_)
      : Processor(handler_)
    {
    }

  protected:
    virtual bool dispatchCall(
      apache::thrift::protocol::TProtocol* in_,
      apache::thrift::protocol::TProtocol* out_,
      const std::string& fname_,
      std::int32_t seqid_,
      void* callContext_) override
    {
      CallContext& ctx = *reinterpret_cast<CallContext*>(callContext_);

      return Processor::dispatchCall(in_, out_, fname_, seqid_, ctx.nextCtx);
    }
  };

public:
  template<class Handler>
  ThriftHandler(Handler *handler_)
    : _processor(std::shared_ptr<Handler>(handler_))
  {
  }

  template<class Handler>
  ThriftHandler(Handler handler_)
    : _processor(handler_)
  {
  }

  std::string key() const override
  {
    return "ThriftHandler";
  }

  std::string beginRequest(const HTTPRequest& req_) override
  {
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::transport;
    using namespace ::apache::thrift::protocol;

    try
    {
      const std::string& content = req_.body;
      LOG(debug) << "Request content:\n" << content;

      std::shared_ptr<TTransport> inputBuffer(
        new TMemoryBuffer((std::uint8_t*)content.c_str(), content.length()));

      std::shared_ptr<TTransport> outputBuffer(new TMemoryBuffer(4096));

      std::shared_ptr<TProtocol> inputProtocol(
        new TJSONProtocol(inputBuffer));
      std::shared_ptr<TProtocol> outputProtocol(
        new TJSONProtocol(outputBuffer));

      CallContext ctx{&req_, nullptr};
      _processor.process(inputProtocol, outputProtocol, &ctx);

      TMemoryBuffer *mBuffer = dynamic_cast<TMemoryBuffer*>(outputBuffer.get());

      std::string response = mBuffer->getBufferAsString();

      LOG(debug)
        << "Response:\n" << response.c_str() << std::endl;

      return response;
    }
    catch (const std::exception& ex)
    {
      LOG(warning) << ex.what();
      throw;
    }
    catch (...)
    {
      LOG(warning) << "Unknown exception has been caught";
      throw;
    }
  }

private:
  LoggingProcessor _processor;
};

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_THRIFTHANDLER_H
