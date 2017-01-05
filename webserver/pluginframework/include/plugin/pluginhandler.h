/*
 * pluginhandler.h
 *
 *  Created on: Mar 22, 2013
 *      Author: ezoltbo
 */

#ifndef PLUGINHANDLER_H_
#define PLUGINHANDLER_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "dynamiclibrary.h"

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
  PluginHandler(int version_) : version(version_) {}

  PluginHandler(int version_, const boost::filesystem::path& path)
    : version(version_)
  {
    addDirectory(path);
  }

  void addDirectory(const boost::filesystem::path& path)
  {
    namespace fs = ::boost::filesystem;

    if (!fs::exists(path) || !fs::is_directory(path))
    {
      throw std::runtime_error(path.string() + " is not a directory");
    }

    fs::directory_iterator endIter;
    for (fs::directory_iterator dirIter(path); dirIter != endIter;
      ++dirIter)
    {
      if (fs::is_regular_file(dirIter->status())
        && fs::extension(*dirIter) == DynamicLibrary::extension())
      {
        std::string dynamicLibraryPath = dirIter->path().string();
        dynamicLibraries.emplace_back(DynamicLibraryPtr(
          new DynamicLibrary(dynamicLibraryPath)));
      }
    }
  }

  boost::program_options::options_description getOptions() const
  {
    namespace po = ::boost::program_options;

    po::options_description desc("Options of plugins");
    for (auto dynamicLibrary : dynamicLibraries)
    {
      typedef po::options_description (*GetOptsFuncPtr)();

      auto getOptions = reinterpret_cast<GetOptsFuncPtr>(
        dynamicLibrary->getSymbol("getOptions"));

      desc.add(getOptions());
    }

    return desc;
  }

  void configure(const boost::program_options::variables_map& configuration)
  {
    namespace po = ::boost::program_options;
    implementationMap.clear();

    for (auto dynamicLibrary : dynamicLibraries)
    {
      typedef void (*RegisterFuncPtr)(const po::variables_map&,
        PluginHandler*);

      auto registerPlugin = reinterpret_cast<RegisterFuncPtr>(
        dynamicLibrary->getSymbol("registerPlugin"));

      registerPlugin(configuration, this);
    }
  }

  BasePtr getImplementation(const std::string& key) const
  {
    auto handleIter = implementationMap.find(key);

    if (handleIter != implementationMap.end())
      return handleIter->second;

    return BasePtr();
  }

  void registerImplementation(const std::string& key, BasePtr implementation,
    int version_)
  {
    if (version != version_)
      throw std::runtime_error("Version mismatch while loading " + key);

    implementationMap[key] = implementation;
  }

  const KeyBasePtrMap& getImplementationMap() const
  {
    return implementationMap;
  }

public:
  static std::string createImplementationKey(
    const std::string& workspaceId_,
    const std::string& serviceName_)
  {
    return workspaceId_ + "/" + serviceName_;
  }

private:
  int version;
  std::vector<DynamicLibraryPtr> dynamicLibraries;
  KeyBasePtrMap implementationMap;
};

} // plugin
} // cc
#endif /* PLUGINHANDLER_H_ */
