/*
 * dynamiclibrary.h
 *
 *  Created on: Mar 21, 2013
 *      Author: ezoltbo
 */

#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include <string>
#include <memory>
#include <sstream>
#include <exception>

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace cc
{
namespace plugin
{

class DynamicLibrary
{
public:
  static std::string extension()
  {
#ifdef WIN32
    return ".dll";
#else
    return ".so";
#endif
  }

  DynamicLibrary(void* handle)
    : handle(handle)
  {
  }

  DynamicLibrary(const std::string& path)
  {
    if (path.empty())
    {
      throw std::runtime_error("Empty path for dynamic library");
    }

    handle = 0;

#ifdef WIN32
    handle = ::LoadLibraryA(name.c_str());
    if (!handle)
    {
      DWORD errorCode = ::GetLastError();
      std::stringstream ss;
      ss << std::string("LoadLibrary(") << path
      << std::string(") Failed. errorCode: ")
      << errorCode;
      throw std::runtime_error(ss.str());
    }
#else
    handle = ::dlopen(path.c_str(), RTLD_NOW);
    if (!handle)
    {
      const char *dlError = ::dlerror();
      std::string error = "Failed to load \"" + path + "\": ";
      error += (dlError) ? dlError : "";
      throw std::runtime_error(error);
    }
#endif
  }

  ~DynamicLibrary()
  {
    if (handle)
    {
#ifdef WIN32
      ::FreeLibrary((HMODULE)handle_);
#else
      ::dlclose(handle);
#endif
    }
  }

  void* getSymbol(const std::string& name) const
  {
    if (!handle)
      throw std::runtime_error("Dynamic library handler is null!");

#ifdef WIN32
    void *ret = ::GetProcAddress((HMODULE)handle_, symbol.c_str());
    if (ret == NULL)
    {
      DWORD errorCode = ::GetLastError();
      std::stringstream ss;
      ss << std::string("getSymbol(") << name
      << std::string(") Failed. errorCode: ")
      << errorCode;
      throw std::runtime_error(ss.str());
    }
#else
    void *ret = ::dlsym(handle, name.c_str());

    if (!ret)
    {
      const char *dlError = ::dlerror();
      std::string error = "Failed to retrieve symbol \"" + name + "\": ";
      error += (dlError) ? dlError : "";

      throw std::runtime_error(error);
    }

#endif
    return ret;
  }

private:
  void* handle;
};

typedef std::shared_ptr<DynamicLibrary> DynamicLibraryPtr;

} // plugin
} // cc

#endif /* DYNAMIC_LIBRARY_H */
