#ifndef CC_WEBSERVER_PLUGINHELPER_H
#define CC_WEBSERVER_PLUGINHELPER_H

#include <memory>

#include <boost/filesystem.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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
  namespace fs = boost::filesystem;
  namespace po = boost::program_options;
  namespace pt = boost::property_tree;

  for (fs::directory_iterator it(vm_["workspace"].as<std::string>());
    it != fs::directory_iterator();
    ++it)
  {
    std::string project = it->path().filename().native();

    fs::path projectInfo = it->path();
    projectInfo += "/project_info.json";
    pt::ptree root;
    pt::read_json(projectInfo.native(), root);

    std::string dbName = root.get<std::string>("database", "");
    if (dbName.empty())
      dbName = project;

    std::string connStr = util::updateConnectionString(
      vm_["database"].as<std::string>(),
      "database",
      dbName);

    std::shared_ptr<odb::database> db = util::createDatabase(connStr);

    if (!db)
    {
      LOG(error)
        << "Wrong connection string: '" << connStr << "' "
        << "for project: '" << project << "' "
        << "for service: '" << serviceName_ << "'";

      throw std::runtime_error("Wrong database!");
    }

    try
    {
      // Create handler
      std::shared_ptr<RequestHandlerT> servicePtr(
        serviceFactory_(
          db,
          std::make_shared<std::string>(fs::canonical(it->path()).native()),
          vm_));

      // Create a key for the implementation
      std::string key = project + '/' + serviceName_;

      // Register implementation
      pluginHandler_->registerImplementation(key, servicePtr);
    }
    catch (const util::ServiceNotAvailException& ex)
    {
      LOG(warning)
        << "Exception: " << ex.what()
        << " in workspace " << project;
    }
  }

  if (pluginHandler_->getImplementationMap().empty())
    throw std::runtime_error(
      "There are no parsed projects in the given workspace directory.");
}

#define CODECOMPASS_SERVICE_FACTORY_WITH_CFG(serviceName, nspace) \
  [](std::shared_ptr<odb::database>& db_, \
     std::shared_ptr<std::string> datadir_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::nspace::serviceName##ServiceProcessor>( \
        new cc::service::nspace::serviceName##ServiceHandler( \
          db_, datadir_, cfg_)); \
  }

#define CODECOMPASS_LANGUAGE_SERVICE_FACTORY_WITH_CFG(serviceName) \
  [](std::shared_ptr<odb::database>& db_, \
     std::shared_ptr<std::string> datadir_, \
     const boost::program_options::variables_map& cfg_) { \
    return new cc::webserver::ThriftHandler< \
      cc::service::language::LanguageServiceProcessor>( \
        new cc::service::language::serviceName##ServiceHandler( \
          db_, datadir_, cfg_)); \
  }

} // plugin
} // cc

#endif // CC_WEBSERVER_PLUGINHELPER_H
