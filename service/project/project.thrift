include "common.thrift"

namespace cpp cc.service.core
namespace java cc.service.core

/*
 * Keep synchronized with grocker::File::type in model/file.h
 */
enum FileType
{
  Unknown = 1,
  GenericFile = 2,
  Directory = 3,

  CSource = 100,
  CxxSource = 101,
  JavaSource = 102,
  DelosSource = 103,
  ErlangSource = 104,

  BashScript = 201,
  PerlScript = 202,
  PythonScript = 203,
  RubyScript = 204,
  SqlScript = 205,
  JavaScript = 206,

  JavaClass = 301
}

enum FileParseStatus
{
  Nothing = 0,
  OnlyInSearchIndex = 1,
  PartiallyParsed = 2,
  FullyParsed = 3,
  VCView = 10000  //dummy for "Version Control View" in the editor
}

struct FileInfo
{
  1:common.FileId file,
  2:string name,
  3:FileType type,
  4:string path,
  
  7:common.FileId parent,  
  
  16:bool hasChildren,
  17:bool isDirectory,
  18:bool isGenerated
  19:FileParseStatus parseStatus;
}

struct ProjectInfo
{
  1:common.ProjectId project,
  2:i64 timestamp,
  3:string name,
  4:string version,
  5:map<string, string> paths
}

struct FileStat 
{
  1:FileInfo info,
  3:string content,

  16:i64 lineCount,
  17:i64 charCount,
  18:i64 byteCount
}

struct BasicFileInfo
{
  1:common.FileId file,
  2:string name,
  4:string path
}

enum MessageType
{
  Unknown,
  Error,
  FatalError,
  Warning,
  Note,
  CodingRule
}

struct BuildLog
{
  1:string message,
  2:MessageType messageType,
  3:common.Range range
}

/**
 * Statistics info
 */
struct StatisticsInfo
{
  1:string group,
  2:string key,
  3:i32    value
}

service ProjectService
{
  FileInfo getFileInfo(1:common.FileId file)
    throws (1:common.InvalidId ex),
  FileInfo getFileInfoByPath(1:string path)
    throws (1:common.InvalidInput ex),
  ProjectInfo getProjectInfo(1:common.ProjectId project)
    throws (1:common.InvalidId ex),
  FileStat getFileStat(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<FileInfo> getRootFiles(1:common.ProjectId project)
    throws (1:common.InvalidId ex),
  list<FileInfo> getChildFiles(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<FileInfo> getSubtree(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<FileInfo> getOpenTreeTillFile(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<FileInfo> getPathTillFile(1:common.FileId file)
    throws (1:common.InvalidId ex),
  FileInfo getContainingFile(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<BuildLog> getBuildLog(1:common.FileId file)
    throws (1:common.InvalidId ex),
  list<FileInfo> searchFile(1:string searchText, 2:bool onlyFile),
  /**
   * Returns a list of project statistics
   */
  list<StatisticsInfo> getStatistics()
}
