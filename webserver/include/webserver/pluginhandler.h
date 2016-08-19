#ifndef CC_WEBSERVER_PLUGINHANDLER_H
#define CC_WEBSERVER_PLUGINHANDLER_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <util/dynamiclibrary.h>

namespace cc
{
namespace webserver
{

template <class Base>
class PluginHandler
{
  typedef std::shared_ptr<Base> BasePtr;
  typedef std::map<std::string, BasePtr> KeyBasePtrMap;

public:
  PluginHandler()
  {
  }

  PluginHandler(const std::string& path_)
  {
    addDirectory(path_);
  }

  void addDirectory(const std::string& path_)
  {
    namespace fs = ::boost::filesystem;

    if (!fs::exists(path_) || !fs::is_directory(path_))
      throw std::runtime_error(path_ + " is not a directory");

    fs::directory_iterator endIter;
    for (fs::directory_iterator dirIter(path_); dirIter != endIter; ++dirIter)
    {
      if (fs::is_regular_file(dirIter->status())
        && fs::extension(*dirIter) == util::DynamicLibrary::extension())
      {
        _dynamicLibraries.emplace_back(util::DynamicLibraryPtr(
          new util::DynamicLibrary(dirIter->path().string())));
      }
    }
  }

  boost::program_options::options_description getOptions() const
  {
    namespace po = ::boost::program_options;

    po::options_description desc("Options of plugins");
    for (util::DynamicLibraryPtr dynamicLibrary : _dynamicLibraries)
    {
      typedef po::options_description (*GetOptsFuncPtr)();

      GetOptsFuncPtr getOptions = reinterpret_cast<GetOptsFuncPtr>(
        dynamicLibrary->getSymbol("getOptions"));

      desc.add(getOptions());
    }

    return desc;
  }

  void configure(const boost::program_options::variables_map& configuration_)
  {
    namespace po = ::boost::program_options;
    _implementationMap.clear();

    for (auto dynamicLibrary : _dynamicLibraries)
    {
      typedef void (*RegisterFuncPtr)(const po::variables_map&,
        PluginHandler*);

      auto registerPlugin = reinterpret_cast<RegisterFuncPtr>(
        dynamicLibrary->getSymbol("registerPlugin"));

      registerPlugin(configuration_, this);
    }
  }

  BasePtr getImplementation(const std::string& key_) const
  {
    auto handleIter = _implementationMap.find(key_);

    if (handleIter != _implementationMap.end())
      return handleIter->second;

    return BasePtr();
  }

  void registerImplementation(const std::string& key_, BasePtr implementation_)
  {
    _implementationMap[key_] = implementation_;
  }

  const KeyBasePtrMap& getImplementationMap() const
  {
    return _implementationMap;
  }

private:
  std::vector<util::DynamicLibraryPtr> _dynamicLibraries;
  KeyBasePtrMap _implementationMap;
};

} // plugin
} // cc

#endif /* CC_PLUGIN_PLUGINHANDLER_H */
