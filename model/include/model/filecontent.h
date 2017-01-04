#ifndef CC_MODEL_FILECONTENT_H
#define CC_MODEL_FILECONTENT_H

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

#pragma db view object(FileContent)
struct FileContentIds
{
  std::string hash;
};

} // model
} // cc

#endif // CC_MODEL_FILECONTENT_H
