include "../../../service/language/language.thrift"
include "../../../service/project/common.thrift"

namespace cpp cc.service.language
namespace netstd language

service CSharpService
{
  language.AstNodeInfo getAstNodeInfo(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  language.AstNodeInfo getAstNodeInfoByPosition(1:common.FilePosition fpos)
    throws (1:common.InvalidInput ex)

  map<string, string> getProperties(1:common.AstNodeId astNodeIds)
    throws (1:common.InvalidId ex)

  string getDocumentation(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

}