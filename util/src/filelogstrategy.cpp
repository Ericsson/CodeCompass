/*
 * filelogstrategy.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: ezoltbo
 */

#include <stdexcept>

#include "util/filelogstrategy.h"

namespace cc
{
namespace util
{

FileLogStrategy::FileLogStrategy(const std::string& filename,
  bool appending)
{
  const char* opentype = appending ? "a" : "w";

  FILE *pFile = fopen(filename.c_str(), opentype);

  if (!pFile)
  {
    throw std::runtime_error("Couldn't open file: " + filename);
  }

  file.reset(pFile, fclose);
}

void FileLogStrategy::writeLog(LogLevel logLevel, const std::string& message)
{
  writeLogMessageOnStream(file.get(), logLevel, message);
}

} // util
} // cc
