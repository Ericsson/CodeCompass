/*
 * standarderrorlogstrategy.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#include <cstdio>

#include "util/standarderrorlogstrategy.h"

namespace cc
{
namespace util
{

void StandardErrorLogStrategy::writeLog(LogLevel logLevel,
  const std::string& message)
{
  writeLogMessageOnStream(stderr, logLevel, message);
}

} // util
} // cc

