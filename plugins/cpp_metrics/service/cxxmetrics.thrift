include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.cppmetrics
namespace java cc.service.cppmetrics

enum CppUnitType
{
  AstNodeUnit = 1,
  FileUnit = 2
}

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

struct CppAllMetricsAstNode
{
  1:common.AstNodeId id,
  2:list<CppMetricsAstNode> metrics
}

struct CppMetricsModule
{
  1:CppModuleMetricsType type,
  2:double value
}

struct CppAllMetricsModule
{
  1:common.FileId id,
  2:list<CppMetricsModule> metrics
}

// Thrift does not provide a union type,
// the ids can remain empty.
struct CppMetricsPath
{
  1:CppUnitType type
  2:common.AstNodeId astNodeId
  3:list<CppMetricsAstNode> astNodeIdMetrics
  4:common.FileId fileId
  5:list<CppMetricsModule> fileMetrics
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
   * This function returns all available C++ metrics
   * (AST node-level) for a particular path.
   */
  list<CppAllMetricsAstNode> getCppAstNodeMetricsForPath(
    1:string path)

  /**
   * This function returns all available C++ metrics
   * (file-level) for a particular path.
   */
  list<CppAllMetricsModule> getCppFileMetricsForPath(
    1:string path)

  /**
   * This function returns the names of AST node metrics.
   */
  list<CppAstNodeMetricsTypeName> getCppAstNodeMetricsTypeNames()

  /**
   * This function returns the names of module-level metrics.
   */
   list<CppModuleMetricsTypeName> getCppModuleMetricsTypeNames()
}