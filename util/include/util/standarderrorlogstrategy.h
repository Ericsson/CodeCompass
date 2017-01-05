/*
 * standarderrorlogstrategy.h
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_STANDARDERRORLOGSTRATEGY_H
#define UTIL_STANDARDERRORLOGSTRATEGY_H

#include <string>

#include "logstrategy.h"

namespace cc 
{
namespace util 
{

class StandardErrorLogStrategy : public LogStrategy
{
public:
  StandardErrorLogStrategy() {}

  void writeLog(LogLevel logLevel, const std::string& message);
};

} // util
} // cc

#endif /* UTIL_STANDARDERRORLOGSTRATEGY_H */
