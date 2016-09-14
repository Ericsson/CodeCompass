#ifndef CC_MODEL_CPPFRIENDSHIP_H
#define CC_MODEL_CPPFRIENDSHIP_H

#include <memory>

namespace cc
{
namespace model
{

#pragma db object
struct CppFriendship
{
  #pragma db id auto
  int id;

  std::uint64_t target;
  std::uint64_t theFriend;

#ifndef NO_INDICES
  #pragma db index member(target)
  #pragma db index member(theFriend)
#endif
};

typedef std::shared_ptr<CppFriendship> CppFriendshipPtr;

}
}

#endif
