#ifndef CC_MODEL_HELM_H
#define CC_MODEL_HELM_H

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>
#include <model/microservice.h>

namespace cc
{
namespace model
{

typedef uint64_t HelmTemplateId;

#pragma db object
struct HelmTemplate
{
  enum class DependencyType
  {
    SERVICE,
    MOUNT,
    CERTIFICATE,
    RESOURCE,
    OTHER
  };

  #pragma db id
  HelmTemplateId id;

  #pragma db not_null
  FileId file;

  std::string name;

  #pragma db not_null
  DependencyType dependencyType;

  #pragma db not_null
  std::string kind;

  #pragma db not_null
  MicroserviceId depends;

  bool operator==(HelmTemplate& rhs);
};
/*
bool HelmTemplate::operator==(HelmTemplate& rhs)
{
  return this->kind == rhs.kind &&
         this->name == rhs.name &&
         this->depends == rhs.depends &&
         this->file == rhs.file &&
         this->dependencyType == rhs.dependencyType;
}*/

inline std::uint64_t createIdentifier(const HelmTemplate& helm_)
{
  return util::fnvHash(
    helm_.name +
    helm_.kind +
    std::to_string(helm_.depends) +
    std::to_string(helm_.file));
}
}
}

#endif // CC_MODEL_HELM_H
