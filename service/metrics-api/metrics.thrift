include "core-api/common.thrift"
include "core-api/project.thrift"

namespace cpp cc.service.metrics
namespace java cc.service.metrics

struct LocInfo
{
  1:project.FileType fileType,
  2:i32 originalLines,
  3:i32 nonblankLines,
  4:i32 codeLines
}

enum MetricsType
{
  OriginalLoc = 1,
  NonblankLoc = 2,
  CodeLoc = 3,
  McCabe = 4,
  CodeChecker = 5
}

struct MetricsTypeName
{
  1:MetricsType type,
  2:string name
}

service MetricsService
{
  /**
   * This function returns a JSON string which represents the file hierarcy with
   * the given file in the root. Every JSON object belongs to a directory or a
   * file. Such an object contains the required metrics given in metricsTypes
   * parameter. The result collects only those files of which the file type is
   * contained by fileTypeFilter.
   */
  string getMetrics(
    1:common.FileId fileId,
    2:list<project.FileType> fileTypeFilter,
    3:MetricsType metricsType)

  /**
   * This function returns the names of metrics.
   */
  list<MetricsTypeName> getMetricsTypeNames()
}
