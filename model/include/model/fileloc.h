#ifndef CODECOMPASS_MODEL_FILE_LOCATION_H
#define CODECOMPASS_MODEL_FILE_LOCATION_H

#include <model/file.h>
#include <model/position.h>

namespace cc
{
namespace model
{

#pragma db value
struct FileLoc
{
  Range range;

  #pragma db null
  odb::lazy_shared_ptr<File> file;
};

} // model
} // cc


#endif // CODECOMPASS_MODEL_FILE_LOCATION_H
