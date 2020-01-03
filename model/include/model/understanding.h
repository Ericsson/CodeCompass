//
// Created by efekane on 2020.01.03..
//

#ifndef PROJECT_UNDERSTANDING_H
#define PROJECT_UNDERSTANDING_H

#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/file.h>

namespace cc
{
namespace model
{
  #pragma db object
  struct Understanding
  {
    #pragma db id auto
    std::uint64_t id;

    #pragma db not_null
    std::shared_ptr<File> file;

    #pragma db not_null
    short ratio;
  };

  typedef std::shared_ptr<Understanding> UnderstandingPtr;

} // model
} // cc

#endif //PROJECT_UNDERSTANDING_H
