#ifndef CODECOMPASS_MODEL_BUILD_PARAMETER_H
#define CODECOMPASS_MODEL_BUILD_PARAMETER_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/buildaction.h>

namespace cc
{
namespace model
{

struct BuildParameter;
struct BuildAction;

typedef std::shared_ptr<BuildParameter> BuildParameterPtr;

#pragma db object
struct BuildParameter
{
  #pragma db id auto
  unsigned long id;

  #pragma db not_null
  std::string param;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_BUILD_PARAMETER_H
