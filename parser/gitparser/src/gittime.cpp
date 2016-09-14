/*
 * gittime.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gittime.h"

#include "git2.h"

namespace cc
{
namespace parser
{

GitTime::GitTime(const git_time& o) : time(o.time), offset(o.offset) { }

} /* namespace parser */
} /* namespace cc */
