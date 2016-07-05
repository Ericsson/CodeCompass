/*
 * dynamiclibrary.h
 *
 *  Created on: Mar 21, 2013
 *      Author: ezoltbo
 */

#ifndef CC_UTIL_DYNAMICLIBRARY_H
#define CC_UTIL_DYNAMICLIBRARY_H

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
namespace util
{

class DynamicLibrary
{
public:
  DynamicLibrary(void* handle_);
  DynamicLibrary(const std::string& path_);
  
  ~DynamicLibrary();

  void* getSymbol(const std::string& name_) const;

  static std::string extension();
private:
  void* _handle;
};

typedef std::shared_ptr<DynamicLibrary> DynamicLibraryPtr;

} // util
} // cc

#endif /* CC_UTIL_DYNAMICLIBRARY_H */
