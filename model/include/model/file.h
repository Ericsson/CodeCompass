#ifndef CORE_FEATURES_FILE_H
#define CORE_FEATURES_FILE_H

#include <string>
#include <memory>
#include <cstdint>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/filecontent.h>
#include <model/project.h>

namespace cc
{
namespace model
{

struct File;
struct Project;
struct FileContent;

typedef odb::lazy_shared_ptr<File> FilePtr;
typedef uint64_t FileId;

#pragma db object
struct File
{
  // Keep synchronized with FileType in project.thrift
  enum Type
  {
    Unknown = 1,
    GenericFile = 2,
    Directory = 3,

    CSource = 100,
    CxxSource = 101,
    JavaSource = 102,
    ErlangSource = 104,

    BashScript = 201,
    PerlScript = 202,
    PythonScript = 203,
    RubyScript = 204,
    SqlScript = 205,
    JavaScript = 206,

    JavaClass = 301
  };
  
  enum ParseStatus
  {
    PSNone = 0,
    PSPartiallyParsed = 1,
    PSFullyParsed = 2,
    PSVCView = 10000, //dummy for "Version Control View" in the editor
  };

  friend class odb::access;

  #pragma db id
  FileId id;

  #pragma db not_null
  Type type = Unknown;

  #pragma db not_null
  std::string path;
  
  #pragma db null
  std::string filename;

  #pragma db null
  std::uint64_t timestamp;

  #pragma db null
  odb::lazy_shared_ptr<FileContent> content;

  #pragma db not_null
  odb::lazy_shared_ptr<Project> project;

  #pragma db null
  odb::lazy_shared_ptr<File> parent;
  
  #pragma db not_null
  ParseStatus parseStatus = PSNone;
  
  bool inSearchIndex = false;

#ifndef NO_INDICES
  #pragma db index member(path)
  #pragma db index member(content)
  #pragma db index member(parent)
#endif

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

#endif // CORE_FEATURES_FILE_H
