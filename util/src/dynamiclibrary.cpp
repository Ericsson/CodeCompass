#include <util/dynamiclibrary.h>

namespace cc
{
namespace util
{

std::string DynamicLibrary::extension()
{
#ifdef WIN32
  return ".dll";
#elif __APPLE__
  return ".dylib";
#else
  return ".so";
#endif
}

DynamicLibrary::DynamicLibrary(void* handle_) : _handle(handle_){}

DynamicLibrary::DynamicLibrary(const std::string& path_)
{
  if (path_.empty())
  {
    throw std::runtime_error("Empty path for dynamic library");
  }

  _handle = 0;

#ifdef WIN32
  _handle = ::LoadLibraryA(name.c_str());
  if (!_handle)
  {
    DWORD errorCode = ::GetLastError();
    std::stringstream ss;
    ss << std::string("LoadLibrary(") << path_
    << std::string(") Failed. errorCode: ")
    << errorCode;
    throw std::runtime_error(ss.str());
  }
#else
  _handle = ::dlopen(path_.c_str(), RTLD_NOW);
  if (!_handle)
  {
    const char *dlError = ::dlerror();
    std::string error = "Failed to load \"" + path_ + "\": ";
    error += (dlError) ? dlError : "";
    throw std::runtime_error(error);
  }
#endif
}

DynamicLibrary::~DynamicLibrary()
{
  if (_handle)
  {
#ifdef WIN32
    ::FreeLibrary((HMODULE)handle_);
#else
    ::dlclose(_handle);
#endif
  }
}

void* DynamicLibrary::getSymbol(const std::string& name_) const
{
  if (!_handle)
    throw std::runtime_error("Dynamic library handler is null!");

#ifdef WIN32
  void *ret = ::GetProcAddress((HMODULE)handle_, symbol.c_str());
  if (ret == NULL)
  {
    DWORD errorCode = ::GetLastError();
    std::stringstream ss;
    ss << std::string("getSymbol(") << name_
    << std::string(") Failed. errorCode: ")
    << errorCode;
    throw std::runtime_error(ss.str());
  }
#else
  void *ret = ::dlsym(_handle, name_.c_str());

  if (!ret)
  {
    const char *dlError = ::dlerror();
    std::string error = "Failed to retrieve symbol \"" + name_ + "\": ";
    error += (dlError) ? dlError : "";

    throw std::runtime_error(error);
  }

#endif
  return ret;
}

} // util
} // cc
