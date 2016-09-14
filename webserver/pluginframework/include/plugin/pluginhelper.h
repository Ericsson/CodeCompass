#ifndef __CODECOMPASS_PLUGIN_HELPER_HPP__
#define __CODECOMPASS_PLUGIN_HELPER_HPP__

#include <mongoose/plugin.h>
#include <mongoose/thrifthandler.h>

#include <plugin/servicenotavailexception.h>

#include <util/util.h>
#include <util/logutil.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <boost/program_options/variables_map.hpp>

namespace cc
{
namespace plugin
{

template <typename RequestHandlerT, typename ServiceFactoryT>
inline void registerPluginSimple(
    const boost::program_options::variables_map&  config_,
    PluginHandler<RequestHandlerT>*               pluginHandler_,
    ServiceFactoryT                               serviceFactory_,
    const std::string&                            serviceName_)
{
  using namespace cc::mongoose;

  const WorkspaceOptions& workspaces =
      config_[WORKSPACE_OPTION_NAME].as<WorkspaceOptions>();

  // Create a handler instance for all workspace
  for (const WorkspaceOption& workspace : workspaces)
  {
    // Create database object
    std::shared_ptr<odb::database> db =
        util::createDatabase(workspace.connectionString);
    if (!db)
    {
      SLog(util::ERROR)
        << "Wrong connection string: '" << workspace.connectionString << "' "
        << "for workspace: '" << workspace.workspaceId << "'"
        << "for service: '" << serviceName_ << "'";

      throw std::runtime_error("Wrong database!");
    }

    boost::program_options::variables_map opts = config_;

    opts.insert(boost::program_options::variables_map::value_type(
      "workspaceId", boost::program_options::variable_value(
        boost::any(workspace.workspaceId), false)));

    if (!workspace.dataDir.empty())
    {
      opts.insert(boost::program_options::variables_map::value_type(
        "datadir", boost::program_options::variable_value(
          boost::any(workspace.dataDir), false)));
    }
    if (!workspace.searchDir.empty())
    {
      opts.insert(boost::program_options::variables_map::value_type(
        "searchdir",  boost::program_options::variable_value(
          boost::any(workspace.searchDir), false)));
    }
    if (!workspace.codeCheckerUrl.empty())
    {
      opts.insert(boost::program_options::variables_map::value_type(
        "codecheckerurl",  boost::program_options::variable_value(
          boost::any(workspace.codeCheckerUrl), false)));
    }
    if (!workspace.codeCheckerRunId.empty())
    {
      opts.insert(boost::program_options::variables_map::value_type(
        "codecheckerrunid",  boost::program_options::variable_value(
          boost::any(workspace.codeCheckerRunId), false)));
    }
    
    if (!workspace.codeCheckerRunName.empty())
    {
      opts.insert(boost::program_options::variables_map::value_type(
        "codecheckerrunname",  boost::program_options::variable_value(
          boost::any(workspace.codeCheckerRunName), false)));
    }

    try{
      // Create handler
      std::shared_ptr<RequestHandlerT> servicePtr(serviceFactory_(db, opts));
      
      // Create a key for the implementation
      std::string key = PluginHandler<RequestHandlerT>::createImplementationKey(
        workspace.workspaceId, serviceName_);
      
      // Register implementation
      pluginHandler_->registerImplementation(
        key, servicePtr, RequestHandlerT::version);
    }catch(ServiceNotAvailException &ex)
    {
          SLog() << "Exception: " << ex.what() <<" in workspace "+ workspace.workspaceId;      
    }
  }
}

#define CODECOMPASS_SIMPLE_SERVICE_FACTORY(serviceName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::mongoose::ThriftHandler< \
      cc::service::nspace::serviceName##ServiceProcessor>( \
        new cc::service::nspace::serviceName##ServiceHandler(db_), \
        cfg_["workspaceId"].as<std::string>()); \
  }

#define CODECOMPASS_SERVICE_FACTORY_WITH_CFG(serviceName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::mongoose::ThriftHandler< \
      cc::service::nspace::serviceName##ServiceProcessor>( \
        new cc::service::nspace::serviceName##ServiceHandler(db_, cfg_), \
        cfg_["workspaceId"].as<std::string>()); \
  }

#define CODECOMPASS_SERVICE_FACTORY_WITH_CFG_NOPOSTFIXSERVICE(serviceName, implName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::mongoose::ThriftHandler< \
      cc::service::nspace::serviceName##Processor>( \
        new cc::service::nspace::implName##ServiceHandler(db_, cfg_), \
        cfg_["workspaceId"].as<std::string>()); \
  }

} // plugin
} // cc

#endif // __CODECOMPASS_PLUGIN_HELPER_HPP__
