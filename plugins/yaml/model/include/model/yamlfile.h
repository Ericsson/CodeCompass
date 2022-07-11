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
struct YamlFile;

typedef std::shared_ptr<YamlFile> YamlFilePtr;

#pragma db object
struct YamlFile
{
  enum Type
  {
    KUBERNETES_CONFIG,
    DOCKER_COMPOSE,
    HELM_CHART,
    HELM_TEMPLATE,
    HELM_VALUES,
    CI,
    OTHER
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  Type type;

  std::string toString() const;
};

inline std::string typeToString(YamlFile::Type type_)
{
  switch (type_)
  {
    case YamlFile::Type::KUBERNETES_CONFIG: return "Kubernetes_config";
    case YamlFile::Type::DOCKER_COMPOSE: return "docker-compose";
    case YamlFile::Type::HELM_CHART: return "Helm chart";
    case YamlFile::Type::HELM_TEMPLATE: return "Helm template";
    case YamlFile::Type::HELM_VALUES: return "Helm values";
    case YamlFile::Type::CI: return "CI";
    case YamlFile::Type::OTHER: return "Other";
  }

  return std::string();
}

inline std::string YamlFile::toString() const
{
  return std::string("YamlFile")
    .append("\nid = ").append(std::to_string(id))
    .append("\nfile id = ").append(std::to_string(file))
    .append("\ntype = ").append(typeToString(type));
}

} //model
} //cc

#endif // CC_MODEL_YAML_H
