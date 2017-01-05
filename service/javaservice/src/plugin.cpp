/*
 * $Id$
 *
 *  Created on: Mar 25, 2013
 *      Author: ezoltbo
 */

#include <plugin/pluginhelper.h>
#include <javaservicehelper/javaservicehelper.h>

#include "javaservice.h"

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Java Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  using namespace cc::mongoose;

  auto factory = [](
    std::shared_ptr<odb::database> db_,
    const boost::program_options::variables_map& cfg_) {
    cc::service::language::JavaServiceHelper javaHelper(db_);
    return new ThriftHandler<cc::service::language::LanguageServiceProcessor>(
      new cc::service::language::JavaServiceHandler(javaHelper),
      cfg_["workspaceId"].as<std::string>());
  };

  cc::plugin::registerPluginSimple(
    configuration,
    pluginHandler,
    factory,
    "JavaService");
}


