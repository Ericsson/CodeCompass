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

struct YamlContent;

typedef std::shared_ptr<YamlContent> YamlContentPtr;

#pragma db object
struct YamlContent
{
  #pragma db id auto
  std::uint64_t id;

  std::string key;

  std::string value;

  #pragma db not_null
  FileId file;

  std::string toString() const;
};

inline std::string YamlContent::toString() const
{
  return std::string("YamlContent")
    .append("\nid = ").append(std::to_string(id))
    .append("\nkey = ").append(key)
    .append("\nvalue = ").append(value)
    .append("\nfile id = ").append(std::to_string(file));
}

} //model
} //cc

#endif // CC_MODEL_YAMLCONTENT_H
