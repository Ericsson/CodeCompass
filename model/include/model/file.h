#ifndef CC_MODEL_FILE_H
#define CC_MODEL_FILE_H

#include <string>
#include <memory>
#include <cstdint>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/filecontent.h>

namespace cc
{
namespace model
{

struct File;
struct Project;
struct FileContent;
struct FileType;

typedef std::shared_ptr<File> FilePtr;
typedef std::shared_ptr<FileType> FileTypePtr;
typedef std::uint64_t FileId;

#pragma db object
struct File
{
  enum ParseStatus
  {
    PSNone = 0,
    PSPartiallyParsed = 1,
    PSFullyParsed = 2,
    PSVCView = 10000, //dummy for "Version Control View" in the editor
  };

  static constexpr const char* DIRECTORY_TYPE = "Directory";
  static constexpr const char* UNKNOWN_TYPE   = "Unknown";

  #pragma db id
  FileId id;

  #pragma db not_null
  std::shared_ptr<FileType> type;

  #pragma db not_null
  std::string path;
  
  #pragma db null
  std::string filename;

  #pragma db null
  std::uint64_t timestamp;

  #pragma db null
  odb::lazy_shared_ptr<FileContent> content;

  #pragma db null
  odb::lazy_shared_ptr<File> parent;
  
  #pragma db not_null
  ParseStatus parseStatus = PSNone;
  
  bool inSearchIndex = false;

#ifndef NO_INDICES
  #pragma db index member(path)
  #pragma db index member(parent)
#endif

};

#pragma db object
struct FileType
{
  FileType(){}
  FileType(std::string name_) : name(name_){}

  #pragma db id auto
  unsigned int id;

  #pragma db unique not_null
  std::string name;
};

#pragma db view object(File)
struct FileIdView
{
  FileId id;
};
  
#pragma db view object(File) \
  query ((?) +  " GROUP BY " + File::parent + " ORDER BY " + File::parent)
struct ParentIdCollector
{
  #pragma db column(File::parent)
  FileId parent;
};
  

} // model
} // cc

#endif // CC_MODEL_FILE_H
