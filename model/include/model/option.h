#ifndef CODECOMPASS_MODEL_OPTION_H
#define	CODECOMPASS_MODEL_OPTION_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/project.h>

namespace cc
{
namespace model
{

struct Option;
typedef std::shared_ptr<Option> OptionPtr;

#pragma db object
struct Option
{
  #pragma db id auto
  unsigned long id;

  #pragma db not_null
  std::string key;
  
  #pragma db null
  std::string value;
  
//  #pragma db not_null
//  std::shared_ptr<Project> project;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_OPTION_H
