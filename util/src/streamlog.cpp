#include <iostream>

#include "util/streamlog.h"

namespace cc
{
namespace util
{

std::string StreamLog::prefix;
std::shared_ptr<LogStrategy> StreamLog::logStrategy;
LogLevel StreamLog::logLevel;

void StreamLog::setStrategy(std::shared_ptr<LogStrategy> strategy)
{
  logStrategy = strategy;
}

void StreamLog::initialize(const std::string& component,
  const std::string& subComponent)
{
  prefix = component + ": " + subComponent + ": ";
}

void StreamLog::setLogLevel(LogLevel logLevel_)
{
  logLevel = logLevel_;
}

LogLevel StreamLog::getLogLevel()
{
  return logLevel;
}

StreamLog::~StreamLog()
{
  if (level < logLevel)
  {
    return;
  }

  std::string message = prefix + os.str();

  if (logStrategy)
  {
    logStrategy->writeLog(level, message);
  } else
  {
    std::cerr
      << "Error: No logging strategy has been set!!!\n"
      << "Anyway, the message was: " << message;
  }
}

} // util
} // cc

