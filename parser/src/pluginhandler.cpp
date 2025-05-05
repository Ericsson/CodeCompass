#include <parser/pluginhandler.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <util/logutil.h>

namespace fs = ::boost::filesystem;

namespace
{
  /**
   * This function returns the real plugin name from path by removing the
   * file extension and the `lib` prefix.
   */
  std::string getPluginName(const boost::filesystem::path& path_)
  {
    // Filename without extension.
    std::string filename = path_.stem().string();
    // Remove "lib" from filename.
    filename.erase(filename.begin(), filename.begin() + 3);

    return filename;
  }
}

namespace cc
{
namespace parser
{

PluginHandler::PluginHandler(const std::string& pluginDir_)
  : _pluginDir(pluginDir_)
{
  if (!fs::exists(_pluginDir) || !fs::is_directory(_pluginDir))
    throw std::runtime_error(_pluginDir + " is not a directory!");
}

void PluginHandler::loadPlugins(std::vector<std::string>& skipParserList_)
{
  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(_pluginDir);
    dirIter != endIter;
    ++dirIter)
  {
    if (fs::is_regular_file(dirIter->status()) &&
        fs::path(*dirIter).extension() == util::DynamicLibrary::extension())
    {
      std::string filename = getPluginName(dirIter->path());

      if (std::find(skipParserList_.begin(), skipParserList_.end(), filename) ==
        skipParserList_.end())
      {
        std::string dynamicLibraryPath = dirIter->path().string();
        _dynamicLibraries[filename] = util::DynamicLibraryPtr(
          new util::DynamicLibrary(dynamicLibraryPath));
      }
      else
      {
        LOG(info) << "[" << filename << "] skipped!";
      }
    }
  }
}

std::vector<std::string> PluginHandler::getLoadedPluginNames() const
{
  std::vector<std::string> plugins;

  for (const auto& lib : _dynamicLibraries)
  {
    plugins.push_back(lib.first);
  }

  return plugins;
}

std::vector<std::string> PluginHandler::getPluginNames() const
{
  std::vector<std::string> plugins;

  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(_pluginDir);
    dirIter != endIter;
    ++dirIter)
  {
    if (fs::is_regular_file(dirIter->status()) &&
        fs::path(*dirIter).extension() == util::DynamicLibrary::extension())
    {
      plugins.push_back(getPluginName(dirIter->path()));
    }
  }

  return plugins;
}

bool PluginHandler::createPlugins(ParserContext& ctx_)
{
  for (const auto& lib : _dynamicLibraries)
  {
    typedef std::shared_ptr<AbstractParser> (*makeParser)(ParserContext& _ctx);
    auto make = reinterpret_cast<makeParser>(lib.second->getSymbol("make"));
    std::shared_ptr<AbstractParser> parser = make(ctx_);
    _parsers[lib.first] = parser;
  }
  return true;
}

boost::program_options::options_description PluginHandler::getOptions() const
{
  namespace po = ::boost::program_options;

  po::options_description desc("Options of plugins");
  for (const auto& lib : _dynamicLibraries)
  {
    typedef po::options_description (*GetOptsFuncPtr)();

    GetOptsFuncPtr getOptions = reinterpret_cast<GetOptsFuncPtr>(
      lib.second->getSymbol("getOptions"));

    desc.add(getOptions());
  }

  return desc;
}

std::shared_ptr<AbstractParser>& PluginHandler::getParser(
  const std::string& parserName_)
{
  return _parsers.at(parserName_);
}

PluginHandler::~PluginHandler()
{
  _parsers.clear();
}

}
}
