include "project/common.thrift"
include "project/project.thrift"
include "language/language.thrift"

namespace cpp cc.service.language
namespace java cc.service.language

typedef string MicroserviceId

enum ServiceType
{
  Internal = 0,
  External = 1
}

struct MicroserviceInfo
{
  1: MicroserviceId serviceId,
  2: string name,
  3: common.FileId fileId
  4: ServiceType type
}

service YamlService extends language.LanguageService
{
  list<MicroserviceInfo> getMicroserviceList()
  /**
   * This function returns a JSON string which represents the file hierarcy with
   * the given file in the root. Every JSON object belongs to a directory or a
   * file. Such an object contains the required metrics given in metricsTypes
   * parameter. The result collects only those files of which the file type is
   * contained by fileTypeFilter.
   */
  /*string getYamlFileDiagram(
    1:common.FileId fileId)

  string getYamlFileInfo(
    1:common.FileId fileId)*/



}
