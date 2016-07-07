#include <util/logutil.h>

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/expressions.hpp>

namespace cc
{
namespace util
{

void initLogger()
{
  auto fmtSeverity = boost::log::expressions::
      attr<boost::log::trivial::severity_level>("Severity");
  boost::log::formatter logFmt =
      boost::log::expressions::format("[%1%] %2%")
      % fmtSeverity % boost::log::expressions::smessage;

  auto fsSink = boost::log::add_console_log(
    std::cout,    
    boost::log::keywords::auto_flush = true
  );
  fsSink->set_formatter(logFmt);
}

} // util
} // cc



