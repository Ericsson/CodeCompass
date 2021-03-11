#ifndef CODECOMPASS_FILECOMMITLOC_H
#define CODECOMPASS_FILECOMMITLOC_H

#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{

#pragma db object
struct FileCommitLoc
{
  #pragma db id
  FileId file;

  #pragma db not_null
  uint16_t totalLines;

  #pragma db not_null
  std::uint64_t commitDate;  // elapsed seconds since 01/01/1970 to commit date
};

typedef std::shared_ptr<FileCommitLoc> FileCommitLocPtr;
}
}


#endif //CODECOMPASS_FILECOMMITLOC_H
