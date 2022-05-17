include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.yaml
namespace java cc.service.yaml


service YamlService
{
  /**
   * This function returns a JSON string which represents the file hierarcy with
   * the given file in the root. Every JSON object belongs to a directory or a
   * file. Such an object contains the required metrics given in metricsTypes
   * parameter. The result collects only those files of which the file type is
   * contained by fileTypeFilter.
   */
  string getYamlFileDiagram(
    1:common.FileId fileId)

  string getYamlFileInfo(
    1:common.FileId fileId)

}
