#ifndef CODECOMPASS_MODEL_FILE_CONTENT_H
#define CODECOMPASS_MODEL_FILE_CONTENT_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{

struct File;
struct FileContent;

typedef std::shared_ptr<FileContent> FileContentPtr;

#pragma db object
struct FileContent
{
  #pragma db id not_null
  std::string hash;

  #pragma db not_null
  std::string content;
};

} // model
} // cc

#endif // CODECOMPASS_MODEL_FILE_CONTENT_H
