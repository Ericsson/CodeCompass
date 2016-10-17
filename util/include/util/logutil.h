#ifndef CC_UTIL_LOGUTIL_H
#define CC_UTIL_LOGUTIL_H

#include <boost/log/trivial.hpp>

namespace cc 
{
namespace util 
{

void initLogger();

boost::log::trivial::severity_level getSeverityLevel();

} // util
} // cc

#endif /* CC_UTIL_LOGUTIL_H */
