#ifndef CC_MODEL_BUILDLOG_H
#define CC_MODEL_BUILDLOG_H

#include <string>
#include <memory>

#include <odb/core.hxx>

#include <model/buildaction.h>
#include <model/buildlogmessage.h>
#include <model/fileloc.h>

namespace cc
{
namespace model
{

struct BuildLog;
typedef std::shared_ptr<BuildLog> BuildLogPtr;

#pragma db object
struct BuildLog
{
  #pragma db id auto
  pktype id;

  #pragma db not_null
  BuildLogMessage log;

  #pragma db not_null
  std::shared_ptr<BuildAction> action;

  #pragma db null
  FileLoc location;
};

} // model
} // cc

#endif // CC_MODEL_BUILDLOG_H
