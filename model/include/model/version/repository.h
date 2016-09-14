#ifndef MODEL_VERSION_REPOSITORY_H_
#define MODEL_VERSION_REPOSITORY_H_

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

struct Repository;
typedef odb::lazy_shared_ptr<Repository> RepositoryPtr;

typedef char Oid[41];


#pragma db object  table("version_repository")
struct Repository
{

#pragma db id auto
  int id;

#pragma db not_null
  std::string path;

#pragma db not_null
  std::string pathHash;

#pragma db not_null
  std::string name;

#pragma db not_null
  bool isHeadDetached;

#pragma db not_null
  std::string head;

  
  friend class odb::access;

};

} //model
} // cc

#endif //MODEL_VERSION_REPOSITORY_H_
