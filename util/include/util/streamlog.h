#ifndef UTIL_STREAMLOG_H
#define UTIL_STREAMLOG_H

#include <sstream>
#include <string>
#include <memory>
#include <thread>
#include <ctime>

#include "logutil.h"
#include "logstrategy.h"

namespace cc 
{
namespace util 
{

class StreamLog
{
public:
  static void setStrategy(std::shared_ptr<LogStrategy> strategy);
  static void initialize(const std::string& component,
    const std::string& subComponent);
  static void setLogLevel(LogLevel logLevel_);
  static LogLevel getLogLevel();

  ~StreamLog();

  std::ostringstream& get(LogLevel level_ = DEBUG)
  {
    level = level_;
    return os;
  }

private:
  static std::string prefix;
  static std::shared_ptr<LogStrategy> logStrategy;
  static LogLevel logLevel;

  std::ostringstream os;
  LogLevel level;
};

} // util
} // cc

#ifndef NO_LOGGING
#define SLog(level) \
    cc::util::StreamLog().get(level) \
      << "[tid: " << std::this_thread::get_id() << "," \
      << " time: " << std::time(nullptr) << "] " \
      << __FILE__ << ':' << __LINE__ << std::endl
#else
#define SLog(level) \
    if (true) ; \
    else \
    cc::util::StreamLog().get(level)
#endif

#endif /* UTIL_STREAMLOG_H */

