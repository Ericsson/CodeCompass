include "../../../service/language/language.thrift"
include "../../../service/project/common.thrift"

namespace cpp cc.service.java
namespace java cc.service.java

service JavaService
{
  language.AstNodeInfo getAstNodeInfoByPosition(1: common.FilePosition fpos)
}