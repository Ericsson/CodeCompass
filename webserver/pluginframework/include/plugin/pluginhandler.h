/*
 * pluginhandler.h
 *
 *  Created on: Mar 22, 2013
 *      Author: ezoltbo
 */

#ifndef CC_PLUGIN_PLUGINHANDLER_H
#define CC_PLUGIN_PLUGINHANDLER_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <util/dynamiclibrary.h>

namespace cc
{
namespace plugin
{

template <class Base>
class PluginHandler
{
  typedef std::shared_ptr<Base> BasePtr;
  typedef std::map<std::string, BasePtr> KeyBasePtrMap;

public:
  PluginHandler(int version_) : _version(version_) {}

  PluginHandler(int version_, const boost::filesystem::path& path_)
    : _version(version_)
  {
    addDirectory(path_);
  }

  void addDirectory(const boost::filesystem::path& path_)
  {
    namespace fs = ::boost::filesystem;

    if (!fs::exists(path_) || !fs::is_directory(path_))
    {
      throw std::runtime_error(path_.string() + " is not a directory");
    }

    fs::directory_iterator endIter;
    for (fs::directory_iterator dirIter(path_); dirIter != endIter;
      ++dirIter)
    {
      if (fs::is_regular_file(dirIter->status())
        && fs::extension(*dirIter) == util::DynamicLibrary::extension())
      {
        std::string dynamicLibraryPath = dirIter->path().string();
        _dynamicLibraries.emplace_back(util::DynamicLibraryPtr(
          new util::DynamicLibrary(dynamicLibraryPath)));        
      }
    }
  }

  boost::program_options::options_description getOptions() const
  {
    namespace po = ::boost::program_options;

    po::options_description desc("Options of plugins");
    for (auto dynamicLibrary : _dynamicLibraries)
    {
      typedef po::options_description (*GetOptsFuncPtr)();

      auto getOptions = reinterpret_cast<GetOptsFuncPtr>(
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
    std::cout << "getImplementation: " << key_ << std::endl;
    auto handleIter = _implementationMap.find(key_);

    if (handleIter != _implementationMap.end())
      return handleIter->second;

    return BasePtr();
  }

  void registerImplementation(const std::string& key_, BasePtr implementation_,
    int version_)
  {
    if (_version != version_)
      throw std::runtime_error("Version mismatch while loading " + key_);

    std::cout << "registerImplementation: " << key_ << std::endl;
    _implementationMap[key_] = implementation_;
  }

  const KeyBasePtrMap& getImplementationMap() const
  {
    return _implementationMap;
  }

public:
  static std::string createImplementationKey(
    const std::string& workspaceId_,
    const std::string& serviceName_)
  {
    return workspaceId_ + "/" + serviceName_;
  }

private:
  int _version;
  std::vector<util::DynamicLibraryPtr> _dynamicLibraries;
  KeyBasePtrMap _implementationMap;
};

} // plugin
} // cc
#endif /* CC_PLUGIN_PLUGINHANDLER_H */
