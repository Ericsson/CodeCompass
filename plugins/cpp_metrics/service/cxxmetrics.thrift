include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.cppmetrics
namespace java cc.service.cppmetrics

enum CppMetricsType
{
  ParameterCount = 1,
  McCabe = 2,
  LackOfCohesion = 3,
  LackOfCohesionHS = 4
}

struct CppMetricsTypeName
{
  1:CppMetricsType type,
  2:string name
}

struct CppMetricsAstNode
{
  1:CppMetricsType type,
  2:double value
}

service CppMetricsService
{
  /**
   * This function returns the required C++ metric
   * for a particular AST node.
   */
  double getSingleCppMetricForAstNode(
    1:common.AstNodeId astNodeId,
    2:CppMetricsType type)

  /**
   * This function returns all available C++ metrics
   * for a particular AST node.
   */
  list<CppMetricsAstNode> getCppMetricsForAstNode(
    1:common.AstNodeId astNodeId)

  /**
   * This function returns the names of metrics.
   */
  list<CppMetricsTypeName> getCppMetricsTypeNames()
}