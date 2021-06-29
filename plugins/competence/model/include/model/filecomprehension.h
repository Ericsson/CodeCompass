#ifndef CC_MODEL_FILECOMPREHENSION_H
#define CC_MODEL_FILECOMPREHENSION_H

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
struct FileComprehension
{
  enum InputType
  {
    REPO,
    USER
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db null
  odb::nullable<float> repoRatio;

  #pragma db not_null
  float userRatio;

  #pragma db not_null
  InputType inputType;

  #pragma db not_null
  std::string userEmail;
};

typedef std::shared_ptr<FileComprehension> FileComprehensionPtr;

} // model
} // cc

#endif //CC_MODEL_FILECOMPREHENSION_H
