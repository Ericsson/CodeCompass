#include <authenticationservice/authenticationservice.h>

#include <boost/program_options.hpp>

#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>
#include <webserver/servercontext.h>
#include <webserver/thrifthandler.h>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description(
      "Authentication Plugin");

    return description;
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    std::shared_ptr<cc::webserver::RequestHandler> handler(
      new cc::webserver::ThriftHandler<
        cc::service::authentication::AuthenticationServiceProcessor>(
        new cc::service::authentication::AuthenticationServiceHandler(
          context_)));

    pluginHandler_->registerImplementation("AuthenticationService", handler);
  }
}
#pragma clang diagnostic pop
