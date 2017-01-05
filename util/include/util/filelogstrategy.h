/*
 * filelogstrategy.h
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_FILELOGSTRATEGY_H
#define UTIL_FILELOGSTRATEGY_H

#include <cstdio>
#include <string>
#include <memory>

#include "logstrategy.h"

namespace cc 
{
namespace util 
{

class FileLogStrategy : public LogStrategy
{
public:
  FileLogStrategy(const std::string& filename,
    bool appending = false);

  void writeLog(LogLevel logLevel, const std::string& message);

private:
  std::shared_ptr<FILE> file;
};

} // util
} // cc

#endif /* UTIL_FILELOGSTRATEGY_H */
