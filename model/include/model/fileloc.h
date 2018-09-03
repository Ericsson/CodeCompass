#ifndef CC_MODEL_FILELOCATION_H
#define CC_MODEL_FILELOCATION_H

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
  #pragma db on_delete(cascade)
  odb::lazy_shared_ptr<File> file;
};

} // model
} // cc


#endif // CC_MODEL_FILELOCATION_H
