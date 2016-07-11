#ifndef CC_MODEL_BUILDLOG_MESSAGE_H
#define CC_MODEL_BUILDLOG_MESSAGE_H

#include <string>

namespace cc
{
namespace model
{

#pragma db value
struct BuildLogMessage
{
  enum MessageType
  {
    Unknown,
    Error,
    FatalError,
    Warning,
    Note,
    CodingRule
  };

  #pragma db not_null
  MessageType type;

  #pragma db not_null
  std::string message;
};

} // model
} // cc

#endif // CC_MODEL_BUILDLOG_MESSAGE_H
