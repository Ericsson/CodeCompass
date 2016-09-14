/*
 * gitexception.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitexception.h"

#include <string>

#include "git2.h"

namespace cc
{
namespace parser
{
  
std::string GitException::helper(int error)
{
  const git_error* errDetails = giterr_last();

  std::string ret = "GitException. Errcode: " + std::to_string(error);

  if (errDetails)
  {
    ret += " Error: ";
    ret += errDetails->message;
  }
  
  return ret;
}
  
GitException::GitException(int error) : runtime_error(helper(error)) { }

  
} /* namespace parser */
} /* namespace cc */
