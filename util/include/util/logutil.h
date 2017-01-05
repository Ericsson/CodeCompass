/*
 * logutil.h
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_LOGUTIL_H
#define UTIL_LOGUTIL_H

#include <cstdio>
#include <string>

namespace cc 
{
namespace util 
{

enum LogLevel
{
  DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, CRITICAL = 4, STATUS = 5
};

const char* const logLevelStringArray[] = {
  "DEBUG",
  "INFO",
  "WARNING",
  "ERROR",
  "CRITICAL",
  "STATUS"
};

void writeLogMessageOnStream(FILE* stream, LogLevel logLevel,
  const std::string& message);

LogLevel getLogLevelFromString(const std::string& loglevel);

} // util
} // cc

#endif /* UTIL_LOGUTIL_H */
