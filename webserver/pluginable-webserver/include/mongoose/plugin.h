/*
 * plugin.h
 *
 *  Created on: Mar 22, 2013
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_PLUGIN_H
#define CODECOMPASS_PLUGIN_H

#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "plugin/pluginhandler.h"
#include "userstatif.h"
#include "mongoose.h"

/**
 * Command line option name.
 */
#define WORKSPACE_OPTION_NAME "workspace"

namespace cc 
{ 
namespace mongoose 
{

struct WorkspaceOption
{
  std::string workspaceId;
  std::string connectionString;
  std::string description;
  std::string dataDir;
  std::string searchDir;
  std::string codeCheckerUrl;
  std::string codeCheckerRunId;
  std::string codeCheckerRunName;
};

using WorkspaceOptions = std::vector<WorkspaceOption>;

class RequestHandler
{
public:
  static const int version = 1;

  virtual std::string key() const = 0;

  virtual int beginRequest(struct mg_connection*, UserStatPtr) = 0;

  virtual ~RequestHandler() {}
};

typedef std::shared_ptr<RequestHandler> RequestHandlerPtr;

} // mongose  
} // cc

// The interface of the plugins:
extern "C" {

boost::program_options::options_description getOptions();

void registerPlugin(
  const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>* pluginHandler);

} // extern "C"

#endif /* PLUGIN_H */
