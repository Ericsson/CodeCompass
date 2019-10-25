#include <boost/program_options.hpp>
#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>
#include <webserver/lsphandler.h>
#include <webserver/servercontext.h>
#include <lspservice/lspservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("LSP Plugin");

    return description;
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    std::shared_ptr<cc::webserver::RequestHandler> handler(
      new cc::webserver::LspHandler(context_));

    pluginHandler_->registerImplementation("LspService", handler);
  }
}
#pragma clang diagnostic pop
