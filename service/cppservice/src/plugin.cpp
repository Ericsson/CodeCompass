/*
 * plugin.cpp
 *
 *  Created on: Mar 25, 2013
 *      Author: ezoltbo
 */

#include <plugin/pluginhelper.h>
#include <cppservicehelper/cppservicehelper.h>

#include "cppservice.h"

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Cpp Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  using namespace cc::mongoose;

  auto facorty = [](
      std::shared_ptr<odb::database> db_,
      const boost::program_options::variables_map& cfg_) {
    cc::service::language::CppServiceHelper cppHelper(db_);
    return new ThriftHandler<cc::service::language::LanguageServiceProcessor>(
      new cc::service::language::CppServiceHandler(cppHelper),
      cfg_["workspaceId"].as<std::string>());
  };

  cc::plugin::registerPluginSimple(
    configuration,
    pluginHandler,
    facorty,
    "CppService");
}


