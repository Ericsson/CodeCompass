#include <util/logutil.h>

#include <boost/log/utility/setup.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>

namespace cc
{
namespace util
{

namespace
{

void consoleLogFormatter(
  const boost::log::record_view& rec, boost::log::formatting_ostream& strm)
{
  auto severity = rec[boost::log::trivial::severity];

  if (severity)
  {
    // Set the color
    switch (severity.get())
    {
      case boost::log::trivial::debug:
        strm << "\033[32m";
        break;
      case boost::log::trivial::warning:
        strm << "\033[33m";
        break;
      case boost::log::trivial::error:
      case boost::log::trivial::fatal:
        strm << "\033[31m";
        break;
      default:
        break;
    }
  }

  std::string sLevel = boost::log::trivial::to_string(severity.get());
  std::transform(sLevel.begin(), sLevel.end(), sLevel.begin(), ::toupper);

  strm << "[" << sLevel << "] " << rec[boost::log::expressions::smessage];

  // Restore the default color
  if (severity)
  {
    strm << "\033[0m";
  }
}

void fileLogFormatter(
  const boost::log::record_view& rec, boost::log::formatting_ostream& strm)
{
  auto severity = rec[boost::log::trivial::severity];

  std::string sLevel = boost::log::trivial::to_string(severity.get());
  std::transform(sLevel.begin(), sLevel.end(), sLevel.begin(), ::toupper);

  strm << "[" << sLevel << "] " << rec[boost::log::expressions::smessage];
}

} // namespace

boost::log::trivial::severity_level getSeverityLevel()
{
 return boost::log::attribute_cast<
   boost::log::attributes::mutable_constant<boost::log::trivial::severity_level>>(
     boost::log::core::get()->get_global_attributes()["Severity"]).get();
}

void initConsoleLogger()
{
  auto fsSink = boost::log::add_console_log(
    std::cout,
    boost::log::keywords::auto_flush = true);

  fsSink->set_formatter(&consoleLogFormatter);
}

bool initFileLogger(const std::string& path_)
{
  auto fsSink = boost::log::add_file_log(
    boost::log::keywords::file_name = path_,
    boost::log::keywords::auto_flush = true
    );
  fsSink->set_formatter(&fileLogFormatter);
  try
  {
    // check if logging to file is possible
    LOG(info) << "Start logging in file: " << path_;
  }
  catch(...)
  {
    boost::log::core::get()->remove_sink(fsSink);
    LOG(warning) << "Could not open file for logging: " << path_;
    return false;
  }
  return true;
}

} // util
} // cc



