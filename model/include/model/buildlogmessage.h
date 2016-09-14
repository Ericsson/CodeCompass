#ifndef CODECOMPASS_MODEL_BUILD_LOG_MESSAGE_H
#define CODECOMPASS_MODEL_BUILD_LOG_MESSAGE_H

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

#endif // CODECOMPASS_MODEL_BUILD_LOG_MESSAGE_H
