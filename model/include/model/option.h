#ifndef CC_MODEL_OPTION_H
#define CC_MODEL_OPTION_H

#include <string>
#include <memory>

#include <odb/core.hxx>

namespace cc
{
namespace model
{

struct Option;
typedef std::shared_ptr<Option> OptionPtr;
typedef int OptionId;

#pragma db object
struct Option
{
  #pragma db id auto
  OptionId id;

  #pragma db not_null
  std::string key;
  
  #pragma db null
  std::string value;  
};

} // model
} // cc

#endif // CC_MODEL_OPTION_H
