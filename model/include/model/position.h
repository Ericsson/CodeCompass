#ifndef CC_MODEL_POSITION_H
#define CC_MODEL_POSITION_H

#include <limits>

namespace cc
{
namespace model
{

#pragma db value
struct Position
{
  typedef std::size_t PosType;

  Position() :
    line(std::numeric_limits<PosType>::max()),
    column(std::numeric_limits<PosType>::max())
  {
  }

  Position(PosType line, PosType column) :
    line(line),
    column(column)
  {
  }

  PosType line;
  PosType column;
};

#pragma db value
struct Range
{
  Range() = default;

  Range(const Position& s, const Position& e): start(s), end(e)  {}

  Position start;
  Position end;
};

} // model
} // cc


#endif // CC_MODEL_POSITION_H
