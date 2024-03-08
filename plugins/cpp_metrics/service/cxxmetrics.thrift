include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.cppmetrics
namespace java cc.service.cppmetrics

enum CppAstNodeMetricsType
{
  ParameterCount = 1,
  McCabe = 2,
  LackOfCohesion = 3,
  LackOfCohesionHS = 4
}

enum CppModuleMetricsType
{
  Placeholder = 1
}

struct CppAstNodeMetricsTypeName
{
  1:CppAstNodeMetricsType type,
  2:string name
}

struct CppModuleMetricsTypeName
{
  1:CppModuleMetricsType type,
  2:string name
}

struct CppMetricsAstNode
{
  1:CppAstNodeMetricsType type,
  2:double value
}

struct CppMetricsModule
{
  1:CppModuleMetricsType type,
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
    2:CppAstNodeMetricsType metric)

  /**
   * This function returns all available C++ metrics
   * for a particular AST node.
   */
  list<CppMetricsAstNode> getCppMetricsForAstNode(
    1:common.AstNodeId astNodeId)

  /**
   * This function returns all available C++ metrics
   * for a particular module.
   */
  list<CppMetricsModule> getCppMetricsForModule(
    1:common.FileId fileId)

  /**
   * This function returns the names of AST node metrics.
   */
  list<CppAstNodeMetricsTypeName> getCppAstNodeMetricsTypeNames()

  /**
   * This function returns the names of module-level metrics.
   */
   list<CppModuleMetricsTypeName> getCppModuleMetricsTypeNames()
}