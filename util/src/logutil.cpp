#include <util/logutil.h>

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/expressions.hpp>

namespace cc
{
namespace util
{

namespace
{
  void logFormatter(
    boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
  {
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
      // Set the color
      switch (severity.get())
      {
        case boost::log::trivial::info:
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

    strm << "[" << rec[boost::log::trivial::severity] << "] "
         << rec[boost::log::expressions::smessage];

    // Restore the default color
    if (severity)
    {
      strm << "\033[0m";
    }
  }
}

void initLogger()
{
  auto fmtSeverity = boost::log::expressions::
      attr<boost::log::trivial::severity_level>("Severity");

  auto fsSink = boost::log::add_console_log(
    std::cout,    
    boost::log::keywords::auto_flush = true
  );

  fsSink->set_formatter(&logFormatter);
}

} // util
} // cc



