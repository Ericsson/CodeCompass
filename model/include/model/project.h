#ifndef CODECOMPASS_MODEL_PROJECT_H
#define CODECOMPASS_MODEL_PROJECT_H

#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{

struct Project;
struct File;

typedef std::shared_ptr<Project> ProjectPtr;

#pragma db object
struct Project
{
  #pragma db id auto
  unsigned long id;

//  #pragma db value_not_null inverse(project)
//  std::vector<odb::lazy_weak_ptr<File>> files;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_PROJECT_H
