#ifndef CC_CODECOMPASS_PLUGIN_H
#define CC_CODECOMPASS_PLUGIN_H

#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <plugin/pluginhandler.h>
#include <mongoose/mongoose.h>

// Command line option name.
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

  virtual int beginRequest(struct mg_connection*) = 0;

  virtual ~RequestHandler() {}
};

typedef std::shared_ptr<RequestHandler> RequestHandlerPtr;

} // mongose  
} // cc

// The interface of the plugins:
extern "C" 
{
	boost::program_options::options_description getOptions();

	void registerPlugin(
		const boost::program_options::variables_map& configuration,
		cc::plugin::PluginHandler<cc::mongoose::RequestHandler>* pluginHandler);

} // extern "C"

#endif /* CC_CODECOMPASS_PLUGIN_H */