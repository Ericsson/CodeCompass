/*
 * position.h
 *
 *  Created on: May 30, 2013
 *      Author: ezoltbo
 */

#ifndef POSITION_H_
#define POSITION_H_

#include <cstddef>
#include <limits>

namespace cc
{
namespace model
{

#pragma db value
struct Position
{
  typedef std::size_t postype;

  Position() :
    line(std::numeric_limits<postype>::max()),
    column(std::numeric_limits<postype>::max())
  {
  }

  Position(postype line, postype column)
    : line(line)
    , column(column)
  {
  }

  postype line;
  postype column;
};

#pragma db value
struct Range
{
  Range() = default;

  Range(const Position& s, const Position& e)
  : start(s), end(e)
  {}

  Position start;
  Position end;
};

} // model
} // cc


#endif /* POSITION_H_ */
