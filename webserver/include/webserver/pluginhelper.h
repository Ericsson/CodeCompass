#ifndef CC_WEBSERVER_PLUGINHELPER_H
#define CC_WEBSERVER_PLUGINHELPER_H

#include <boost/program_options/variables_map.hpp>
#include <boost/log/trivial.hpp>

#include <odb/database.hxx>

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

    po::variables_map vm = vm_;
    vm.insert(std::make_pair("datadir", po::variable_value(
      wsOpt.datadir, false)));

    std::shared_ptr<odb::database> db
      = util::createDatabase(wsOpt.connectionString);

    if (!db)
    {
      BOOST_LOG_TRIVIAL(error)
        << "Wrong connection string: '" << wsOpt.connectionString << "' "
        << "for workspace: '" << wsId << "' "
        << "for service: '" << serviceName_ << "'";

      throw std::runtime_error("Wrong database!");
    }

    try
    {
      // Create handler
      std::shared_ptr<RequestHandlerT> servicePtr(serviceFactory_(db, vm));
      
      // Create a key for the implementation
      std::string key = wsId + '/' + serviceName_;
      
      // Register implementation
      pluginHandler_->registerImplementation(key, servicePtr);
    }
    catch (const util::ServiceNotAvailException& ex)
    {
      BOOST_LOG_TRIVIAL(warning)
        << "Exception: " << ex.what()
        << " in workspace " << wsId;
    }
  }
}

#define CODECOMPASS_SERVICE_FACTORY_WITH_CFG(serviceName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::nspace::serviceName##ServiceProcessor>( \
        new cc::service::nspace::serviceName##ServiceHandler(db_, cfg_), \
        cfg_["workspace"].as<std::string>()); \
  }

#define CODECOMPASS_LANGUAGE_SERVICE_FACTORY_WITH_CFG(serviceName) \
  [](std::shared_ptr<odb::database>& db_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::language::LanguageServiceProcessor>( \
        new cc::service::language::serviceName##ServiceHandler(db_, cfg_), \
        cfg_["workspace"].as<std::string>()); \
  }

} // plugin
} // cc

#endif // CC_WEBSERVER_PLUGINHELPER_H
