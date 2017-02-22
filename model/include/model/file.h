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

typedef std::shared_ptr<File> FilePtr;
typedef std::uint64_t FileId;

#pragma db object
struct File
{
  enum ParseStatus
  {
    PSNone = 0,
    PSPartiallyParsed = 1,
    PSFullyParsed = 2
  };

  static constexpr const char* DIRECTORY_TYPE = "Dir";
  static constexpr const char* UNKNOWN_TYPE   = "Unknown";

  #pragma db id
  FileId id;

  #pragma db not_null type("VARCHAR(8)")
  std::string type;

  #pragma db not_null
  std::string path;
  
  #pragma db null
  std::string filename;

  #pragma db null
  std::uint64_t timestamp;

  // The reason why the file content attribute is read-only is that when a
  // plugin adds a file through the SourceManager then the content is also added
  // for the first time. When another plugin wants to modify the file type for
  // example then it calls the SourceManager::updateFile() function. If the file
  // object after the update doesn't contain the content because of its lazyness
  // then the content field in the table for this file becomes null.
  // If the source code will be editable sometime then this read-only state
  // should be reconsidered.
  #pragma db null readonly
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

#pragma db view object(File)
struct FileIdView
{
  FileId id;
};
  
#pragma db view object(File) query((?) + " GROUP BY " + File::type)
struct FileTypeView
{
  std::string type;
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
