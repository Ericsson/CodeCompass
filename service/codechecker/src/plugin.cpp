/*
 * plugin.cpp
 *
 *  Created on: Mar 25, 2013
 *      Author: ezoltbo
 */

#include <plugin/pluginhelper.h>

#include "codecheckerproxy.h"

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("CodeChecker Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  cc::plugin::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_SERVICE_FACTORY_WITH_CFG_NOPOSTFIXSERVICE(codeCheckerDBAccess, CodeCheckerProxy, codechecker),
    "CodeCheckerService");
}

