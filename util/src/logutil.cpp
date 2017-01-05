/*
 * logutil.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#include "util/logutil.h"

namespace cc
{
namespace util
{

void writeLogMessageOnStream(FILE* stream, LogLevel logLevel,
  const std::string& message)
{
  std::string msg = std::string(logLevelStringArray[logLevel]) + ' '
    + message;

  if (msg[msg.size() - 1] != '\n')
  {
    msg += '\n';
  }

  fprintf(stream, "%s", msg.c_str());
  fflush(stream);
}

LogLevel getLogLevelFromString(const std::string& loglevel)
{
  if (loglevel=="debug")
    return DEBUG;
  if (loglevel=="info")
    return INFO;
  if (loglevel=="warning")
    return WARNING;
  if (loglevel=="error")
    return ERROR;
  if (loglevel=="critical")
    return CRITICAL;
  if (loglevel=="status")
    return STATUS;

  return DEBUG;
}

} // util
} // cc



