#ifndef CC_MODEL_YAMLCONTENT_H
#define CC_MODEL_YAMLCONTENT_H

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>
#include <model/yamlastnode.h>

namespace cc
{
namespace model
{

#pragma db object
struct YamlContent
{
  #pragma db id auto
  std::uint64_t id;

  //#pragma db not_null
  YamlAstNodeId key;

  //#pragma db not_null
  YamlAstNodeId value;

  //#pragma db
  YamlAstNodeId parent;

  #pragma db not_null
  FileId file;
};


} //model
} //cc

#endif // CC_MODEL_YAMLCONTENT_H
