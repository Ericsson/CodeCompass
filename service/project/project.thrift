include "common.thrift"

namespace cpp cc.service.core
namespace java cc.service.core

enum FileParseStatus
{
  Nothing = 0,
  OnlyInSearchIndex = 1,
  PartiallyParsed = 2,
  FullyParsed = 3
}

struct FileInfo
{
  1:common.FileId id,
  2:string name,
  3:string type,
  4:string path,
  5:common.FileId parent,  
  6:bool hasChildren,
  7:bool isDirectory,
  8:FileParseStatus parseStatus;
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

struct StatisticsInfo
{
  1:string group,
  2:string key,
  3:i32 value
}

service ProjectService
{
  FileInfo getFileInfo(1:common.FileId fileId)
    throws (1:common.InvalidId ex),
  FileInfo getFileInfoByPath(1:string path)
    throws (1:common.InvalidInput ex),
  string getFileContent(1:common.FileId fileId)
    throws (1:common.InvalidId ex),
  FileInfo getParent(1:common.FileId fileId)
    throws (1:common.InvalidId ex),
  list<FileInfo> getRootFiles(),
  list<FileInfo> getChildFiles(1:common.FileId fileId),
  list<FileInfo> getSubtree(1:common.FileId fileId)
  list<FileInfo> getOpenTreeTillFile(1:common.FileId fileId),
  list<FileInfo> getPathTillFile(1:common.FileId fileId),
  list<BuildLog> getBuildLog(1:common.FileId fileId),
  list<FileInfo> searchFile(1:string text, 2:bool onlyFile),
  list<StatisticsInfo> getStatistics()
  list<string> getFileTypes()
}
