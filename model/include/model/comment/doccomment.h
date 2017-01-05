#ifndef MODEL_DOCCOMMENT_H_
#define MODEL_DOCCOMMENT_H_

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

struct DocComment;
typedef odb::lazy_shared_ptr<DocComment> DocCommentPtr;


#pragma db object
struct DocComment
{

#pragma db id auto
  int id;

#pragma db not_null index
  unsigned long long contentHash;

#pragma db not_null
  std::string contentHTML;

#pragma db not_null index
  unsigned long long mangledNameHash;

  friend class odb::access;
  
};

} //model
} // cc

#endif //MODE_DOCCOMMENT_H_
