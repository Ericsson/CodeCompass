//
// Created by efekane on 2020.01.03..
//

#ifndef CC_MODEL_FILECOMPREHENSION_H
#define CC_MODEL_FILECOMPREHENSION_H

#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{
  #pragma db object
  struct FileComprehension
  {
    #pragma db id auto
    std::uint64_t id;

    #pragma db not_null
    std::shared_ptr<File> file;

    #pragma db not_null
    short ratio;
  };

  typedef std::shared_ptr<FileComprehension> FileComprehensionPtr;

} // model
} // cc

#endif //CC_MODEL_FILECOMPREHENSION_H
