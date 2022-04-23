#ifndef CC_MODEL_YAMLCONTENT_H
#define CC_MODEL_YAMLCONTENT_H

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{

#pragma db object
struct YamlContent
{
  #pragma db not_null
  std::string key;

  #pragma db not_null
  std::string data;

  #pragma db not_null
  FileId file;

  #pragma db id auto
  std::uint64_t id;
};


} //model
} //cc

#endif // CC_MODEL_YAMLCONTENT_H
