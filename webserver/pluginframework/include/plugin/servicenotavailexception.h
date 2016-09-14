#ifndef __SERVICENOTAVAILEXCEPTION_H__
#define __SERVICENOTAVAILEXCEPTION_H__

#include <stdexcept>

class ServiceNotAvailException: public std::runtime_error{
public:
  explicit ServiceNotAvailException(const std::string& what_arg_)
    : std::runtime_error(what_arg_)
  {
  }
};

#endif
