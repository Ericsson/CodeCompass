#ifndef CC_MODEL_CPPDOCCOMMENT_H
#define CC_MODEL_CPPDOCCOMMENT_H

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

struct CppDocComment;
typedef odb::lazy_shared_ptr<CppDocComment> CppDocCommentPtr;

#pragma db object
struct CppDocComment
{

  #pragma db id auto
  int id;

  #pragma db not_null index
  unsigned long long contentHash;

  #pragma db not_null
  std::string content;

  #pragma db not_null index
  unsigned long long mangledNameHash;
};

} //model
} // cc

#endif //CC_MODEL_CPPDOCCOMMENT_H
