#include "LspService.h"
#include <boost/program_options.hpp>
#include <webserver/pluginhelper.h>
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
    cc::webserver::registerLspPluginSimple(
      context_,
      pluginHandler_,
      CODECOMPASS_LSP_SERVICE_FACTORY_WITH_CFG(Lsp, lsp),
      "LspService");
  }
}
#pragma clang diagnostic pop
