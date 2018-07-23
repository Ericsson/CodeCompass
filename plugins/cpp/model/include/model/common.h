#ifndef CC_MODEL_COMMON_H
#define CC_MODEL_COMMON_H

#include <string>

namespace cc
{
namespace model
{

enum Visibility
{
  Private,
  Protected,
  Public
};

enum Tag
{
  Constructor = 0,
  Destructor = 1,
  Virtual = 2,
  Static  = 3,
  Implicit = 4,
  Global = 5,
  Constant = 6,
  UserDeleted = 7
};

inline std::string visibilityToString(Visibility v_)
{
  return
    v_ == Private   ? "private" :
    v_ == Protected ? "protected" :
    v_ == Public    ? "public" : "";
}

inline std::string tagToString(Tag t_)
{
  return
    t_ == Constructor   ? "constructor" :
    t_ == Destructor    ? "destructor" :
    t_ == Virtual       ? "virtual" :
    t_ == Static        ? "static" :
    t_ == Implicit      ? "implicit" :
    t_ == Global        ? "global" :
    t_ == Constant      ? "constant" :
    t_ == UserDeleted   ? "user-deleted" : "";
}

}
}

#endif
