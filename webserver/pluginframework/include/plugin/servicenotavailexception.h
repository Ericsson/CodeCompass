#ifndef CC_PLUGIN_SERVICENOTAVAILEXCEPTION_H
#define CC_PLUGIN_SERVICENOTAVAILEXCEPTION_H

#include <stdexcept>

class ServiceNotAvailException: public std::runtime_error{
public:
  explicit ServiceNotAvailException(const std::string& what_arg_)
    : std::runtime_error(what_arg_)
  {
  }
};

#endif // CC_PLUGIN_SERVICENOTAVAILEXCEPTION_H
