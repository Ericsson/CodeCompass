// $Id$

#ifndef MODEL_JAVA_JAVADOCCOMMENT_H
#define MODEL_JAVA_JAVADOCCOMMENT_H

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

struct JavaDocComment;
typedef odb::lazy_shared_ptr<JavaDocComment> JavaDocCommentPtr;


#pragma db object
struct JavaDocComment
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

#endif //MODEL_JAVA_JAVADOCCOMMENT_H
