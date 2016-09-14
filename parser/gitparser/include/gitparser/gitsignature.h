/*
 * gitsignature.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITSIGNATURE_H
#define GITSIGNATURE_H

#include <utility>
#include <string>

#include "gittime.h"

struct git_signature;

namespace cc
{
namespace parser
{
  
class GitSignature
{
  std::string name;
  std::string email;
  GitTime when;
  
public:
  GitSignature(){};
  GitSignature(const git_signature *sig);
  
  const std::string &getName() const
  {
    return name;
  }
  
  const std::string &getEmail() const
  {
    return email;
  }
  
  const GitTime &getTime() const
  {
    return when;
  }
  
  std::string toString() const
  {
    return name + " <" + email + ">";
  }
  
  
};

} /* namespace parser */
} /* namespace cc */

#endif // GITSIGNATURE_H
