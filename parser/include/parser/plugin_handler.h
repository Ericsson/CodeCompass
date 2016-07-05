#ifndef CC_PARSER_PLUGINHANDLER_H
#define CC_PARSER_PLUGINHANDLER_H

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <parser/plugin_handler.h>
#include <parser/abstract_parser.h>
#include <util/dynamiclibrary.h>

namespace cc
{
namespace parser
{

class PluginHandler
{
public:
  PluginHandler(ParserContext& ctx_);
  ~PluginHandler();
  bool loadPluginsFromDir(const std::string& path_);  
  std::vector<std::string> getTopologicalOrder();      
  std::shared_ptr<AbstractParser>& getParser(std::string parserName_);  
private:
  std::map<std::string, std::shared_ptr<AbstractParser> > _parsers;
  std::vector<util::DynamicLibraryPtr> _dynamicLibraries;
  ParserContext& _ctx;
};

} // plugin
} // cc
#endif /* PLUGINHANDLER_H */
