#ifndef CC_WEBSERVER_PLUGINHELPER_H
#define CC_WEBSERVER_PLUGINHELPER_H

#include <memory>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <util/logutil.h>
#include <util/util.h>
#include <util/dbutil.h>
#include <util/webserverutil.h>

#include "requesthandler.h"
#include "thrifthandler.h"

namespace cc
{
namespace webserver
{

template <typename RequestHandlerT, typename ServiceFactoryT>
inline void registerPluginSimple(
  const boost::program_options::variables_map& vm_,
  PluginHandler<RequestHandlerT>* pluginHandler_,
  ServiceFactoryT serviceFactory_,
  const std::string& serviceName_)
{
  namespace po = boost::program_options;

  const util::WorkspaceOptions workspaces
    = util::parseConfigFile(vm_["workspaceCfgFile"].as<std::string>());

  // Create a handler instance for all workspaces.
  for (const auto& workspace : workspaces)
  {
    const std::string wsId = workspace.first;
    const util::WorkspaceOption& wsOpt = workspace.second;

    std::shared_ptr<odb::database> db
      = util::createDatabase(wsOpt.connectionString);

    if (!db)
    {
      LOG(error)
        << "Wrong connection string: '" << wsOpt.connectionString << "' "
        << "for workspace: '" << wsId << "' "
        << "for service: '" << serviceName_ << "'";

      throw std::runtime_error("Wrong database!");
    }

    try
    {
      // Create handler
      std::shared_ptr<RequestHandlerT> servicePtr(
        serviceFactory_(db, std::make_shared<std::string>(wsOpt.datadir), vm_));

      // Create a key for the implementation
      std::string key = wsId + '/' + serviceName_;

      // Register implementation
      pluginHandler_->registerImplementation(key, servicePtr);
    }
    catch (const util::ServiceNotAvailException& ex)
    {
      LOG(warning)
        << "Exception: " << ex.what()
        << " in workspace " << wsId;
    }
  }
}

#define CODECOMPASS_SERVICE_FACTORY_WITH_CFG(serviceName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     std::shared_ptr<std::string> datadir_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::nspace::serviceName##ServiceProcessor>( \
        new cc::service::nspace::serviceName##ServiceHandler( \
          db_, datadir_, cfg_), cfg_["workspace"].as<std::string>()); \
  }

#define CODECOMPASS_LANGUAGE_SERVICE_FACTORY_WITH_CFG(serviceName) \
  [](std::shared_ptr<odb::database>& db_, \
     std::shared_ptr<std::string> datadir_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::language::LanguageServiceProcessor>( \
        new cc::service::language::serviceName##ServiceHandler( \
          db_, datadir_, cfg_), cfg_["workspace"].as<std::string>()); \
  }

} // plugin
} // cc

#endif // CC_WEBSERVER_PLUGINHELPER_H
