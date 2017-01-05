/*
 * gitexception.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITEXCEPTION_H
#define GITEXCEPTION_H

#include <stdexcept>

namespace cc
{
namespace parser
{

/**
 * An exception of this class is thrown when an underlying libgit2 function
 * return an error code.
 */
class GitException: public std::runtime_error
{
  std::string helper(int error);
  
public:
  explicit GitException(int error);
  
  static void hadleError(int error)
  {
    if (error) {
      throw GitException(error);
    }
  }
};

} /* namespace parser */
} /* namespace cc */


#endif // GITEXCEPTION_H
