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

inline std::string visibilityToString(Visibility v_)
{
  return
    v_ == Private   ? "private" :
    v_ == Protected ? "protected" :
    v_ == Public    ? "public" : "";
}

}
}

#endif
