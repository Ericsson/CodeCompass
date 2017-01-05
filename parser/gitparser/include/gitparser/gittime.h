/*
 * gittime.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITTIME_H
#define GITTIME_H

#include <cstdint>

struct git_time;

namespace cc
{
namespace parser
{

/**
 * Class to store a time point.
 * 
 * Corresponding type for libgit2's <tt>git_time</tt>
 */
class GitTime
{
  int64_t time;
  int offset;
  
public:
  GitTime() : time(0), offset(0) {};
  GitTime(const git_time&);
  
  int64_t getTime() const
  {
    return time;
  }
  
  int getOffset() const
  {
    return offset;
  }
  
};

} /* namespace parser */
} /* namespace cc */

#endif // GITTIME_H
