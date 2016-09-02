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

  Position(PosType line, PosType column) : line(line), column(column)
  {
  }

  PosType line;
  PosType column;
};

inline bool operator<(const Position& lhs, const Position& rhs)
{
  return lhs.line < rhs.line ||
    (lhs.line == rhs.line && lhs.column < rhs.column);
}

inline bool operator==(const Position& lhs, const Position& rhs)
{
  return lhs.line == rhs.line && lhs.column == rhs.column;
}

inline bool operator!=(const Position& lhs, const Position& rhs)
{
  return !(lhs == rhs);
}

#pragma db value
struct Range
{
  Range() = default;

  Range(const Position& s, const Position& e) : start(s), end(e)  {}

  Position start;
  Position end;
};

/**
 * Range lhs is less than range rhs if rhs is completely contains lhs.
 */
inline bool operator<(const Range& lhs, const Range& rhs)
{
  if (lhs.start == rhs.start)
    return lhs.end < rhs.end;

  if (lhs.end == rhs.end)
    return rhs.start < lhs.start;

  return rhs.start < lhs.start && lhs.end < rhs.end;
}

inline bool operator==(const Range& lhs, const Range& rhs)
{
  return lhs.start == rhs.start && lhs.end == rhs.end;
}

inline bool operator!=(const Range& lhs, const Range& rhs)
{
  return !(lhs == rhs);
}

} // model
} // cc

#endif // CC_MODEL_POSITION_H
