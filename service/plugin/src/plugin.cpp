#include <webserver/requesthandler.h>
#include <webserver/servercontext.h>
#include <webserver/thrifthandler.h>
#include <pluginservice/pluginservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Plugin Plugin");

    return description;
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    std::shared_ptr<cc::webserver::RequestHandler> handler(
      new cc::webserver::ThriftHandler<cc::service::plugin::PluginServiceProcessor>(
        new cc::service::plugin::PluginServiceHandler(pluginHandler_,
                                                      context_)));

    pluginHandler_->registerImplementation("PluginService", handler);
  }
}
#pragma clang diagnostic pop
