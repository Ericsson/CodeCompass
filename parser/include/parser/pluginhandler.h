#ifndef CC_PARSER_PLUGINHANDLER_H
#define CC_PARSER_PLUGINHANDLER_H

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <parser/pluginhandler.h>
#include <parser/abstractparser.h>
#include <util/dynamiclibrary.h>

namespace cc
{
namespace parser
{

class PluginHandler
{
public:
  /**
   * Parser plugin handler constructor.
   * @param pluginDir_ Parser plugin directory. The parser plugins as .so files
   * are located in this directory.
   */
  PluginHandler(const std::string& pluginDir_);

  /**
   * Plugin handler destructor.
   */
  ~PluginHandler();

  /**
   * Load plugins from parser directory.
   * @param skipParserList_ These parsers will be skipped.
   */
  void loadPlugins(std::vector<std::string>& skipParserList_);

  /**
   * Return the list of available plugins based on the file names in the parser
   * lib directory.
   */
  std::vector<std::string> getPluginNames() const;

  /**
   * Create parser plugins from shared object by calling make() method.
   * @param ctx_ Parser context.
   */
  bool createPlugins(ParserContext& ctx_);

  /**
   * Get parser by parser name.
   * @param parserName_ Parser name (e.g.: dummyparser, cppparser). A parser is
   * identified by its name. For example dummyparser is implemented in
   * libdummyparser.so.
   */
  std::shared_ptr<AbstractParser>& getParser(
    const std::string& parserName_);

  /**
   * Get arguments for each parsers. (e.g.: dummy-arg)
   */
  boost::program_options::options_description getOptions() const;

private:
  std::map<std::string, std::shared_ptr<AbstractParser> > _parsers;
  std::map<std::string, util::DynamicLibraryPtr> _dynamicLibraries;
  const std::string& _pluginDir;
};

} // plugin
} // cc

#endif /* CC_PARSER_PLUGINHANDLER_H */
