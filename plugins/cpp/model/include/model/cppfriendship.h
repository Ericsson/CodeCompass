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

  std::string toString() const
  {
    return std::string("CppFriendship")
      .append("\nid = ").append(std::to_string(id))
      .append("\ntarget = ").append(std::to_string(target))
      .append("\ntheFriend = ").append(std::to_string(theFriend));
  }

#ifndef NO_INDICES
  #pragma db index member(target)
  #pragma db index member(theFriend)
#endif
};

typedef std::shared_ptr<CppFriendship> CppFriendshipPtr;

#pragma db view object(CppFriendship)
struct CppFriendshipCount
{
  #pragma db column("count(" + CppFriendship::id + ")")
  std::size_t count;
};

}
}

#endif
