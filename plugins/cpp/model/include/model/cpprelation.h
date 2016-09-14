#ifndef CC_MODEL_CPPRELATION_H
#define CC_MODEL_CPPRELATION_H

#include <memory>

namespace cc
{
namespace model
{

#pragma db object
struct CppRelation
{
  enum class Kind
  {
    Override,
    Alias,
    Assign,
    DeclContext
  };

  #pragma db id auto
  int id;

  std::uint64_t lhs;
  std::uint64_t rhs;

  Kind kind;

#ifndef NO_INDICES
  #pragma db index member(lhs)
  #pragma db index member(rhs)
#endif
};

typedef std::shared_ptr<CppRelation> CppRelationPtr;

}
}

#endif
