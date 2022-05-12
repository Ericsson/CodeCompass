include "../../../service/language/language.thrift"
include "../../../service/project/common.thrift"

namespace cpp cc.service.csharp
namespace netstd csharp

service CsharpService
{
  language.AstNodeInfo getAstNodeInfo(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  language.AstNodeInfo getAstNodeInfoByPosition(1:string path, 2:common.Position fpos)
    throws (1:common.InvalidInput ex)

  map<string, string> getProperties(1:common.AstNodeId astNodeIds)
    throws (1:common.InvalidId ex)

  string getDocumentation(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)
  
  common.FileRange getFileRange(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  map<string, i32> getReferenceTypes(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  i32 getReferenceCount(
    1:common.AstNodeId astNodeId,
    2:i32 referenceId)

  list<language.AstNodeInfo> getReferences(
    1:common.AstNodeId astNodeId,
    2:i32 referenceId
    3:list<string> tags)
      throws (1:common.InvalidId ex)

  map<string, i32> getFileReferenceTypes()
    throws (1:common.InvalidId ex)

  i32 getFileReferenceCount(
    1:string path,
    2:i32 referenceId)

  list<language.AstNodeInfo> getFileReferences(
    1:string path,
    2:i32 referenceId)
      throws (1:common.InvalidId ex)

  map<string, i32> getDiagramTypes(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  string getDiagram(1:common.AstNodeId astNodeId, 2:i32 diagramId)
    throws (1:common.InvalidId exId, 2:common.Timeout exLong)

  list<language.SyntaxHighlight> getSyntaxHighlight(
   1:common.FileRange range,
   2:list<string> content)

}