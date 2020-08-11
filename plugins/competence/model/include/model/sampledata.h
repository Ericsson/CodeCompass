#ifndef CODECOMPASS_SAMPLEDATA_H
#define CODECOMPASS_SAMPLEDATA_H

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
struct SampleData
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  uint32_t occurrences;
};

typedef std::shared_ptr<SampleData> SampleDataPtr;
}
}


#endif //CODECOMPASS_SAMPLEDATA_H
