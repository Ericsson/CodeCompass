#ifndef CC_MODEL_YAML_H
#define CC_MODEL_YAML_H

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
struct Yaml
{
  enum Type
  {
    HELM_CHART,
    OTHER
  };


  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  Type type;
};

} //model
} //cc

#endif // CC_MODEL_YAML_H
