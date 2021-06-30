#ifndef CODECOMPASS_COMMITDATA_H
#define CODECOMPASS_COMMITDATA_H

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
struct CommitData
{
  #pragma db id auto
  std::uint64_t id;

  //#pragma db not_null
  //std::string commit_id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  std::string committerEmail;  // TODO: rename this to authorEmail

  #pragma db not_null
  uint16_t committedLines;

  #pragma db not_null
  uint16_t totalLines;

  #pragma db not_null
  uint16_t commitType;

  #pragma db not_null
  std::uint64_t commitDate;  // elapsed seconds since 01/01/1970 to commit date
};

typedef std::shared_ptr<CommitData> CommitDataPtr;
}
}


#endif //CODECOMPASS_COMMITDATA_H
