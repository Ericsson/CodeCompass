#ifndef CC_MODEL_BUILDLOG_H
#define CC_MODEL_BUILDLOG_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/buildaction.h>
#include <model/fileloc.h>

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

#pragma db object
struct BuildLog
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  BuildLogMessage log;

  #pragma db null
  FileLoc location;
};

typedef std::shared_ptr<BuildLog> BuildLogPtr;

} // model
} // cc

#endif // CC_MODEL_BUILDLOG_H
