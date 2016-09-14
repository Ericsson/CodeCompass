/*
 * logstrategy.h
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_LOGSTRATEGY_H
#define UTIL_LOGSTRATEGY_H

#include "logutil.h"

namespace cc 
{
namespace util 
{

class LogStrategy
{
public:
  virtual void writeLog(LogLevel logLevel, const std::string& message) = 0;

  virtual ~LogStrategy() {}

protected:
  LogStrategy() {}
};

} // util
} // cc

#endif /* UTIL_LOGSTRATEGY_H */
