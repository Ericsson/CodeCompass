include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.cppmetrics
namespace java cc.service.cppmetrics

enum CppAstNodeMetricsType
{
  ParameterCount = 1,
  McCabe = 2,
  BumpyRoad = 3,
  LackOfCohesion = 4,
  LackOfCohesionHS = 5,
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

struct CppMetricsAstNodeSingle
{
  1:string path,
  2:CppAstNodeMetricsType type,
  3:double value
}

struct CppMetricsAstNodeAll
{
  1:common.AstNodeId id,
  2:list<CppMetricsAstNodeSingle> metrics
}

struct CppMetricsModuleSingle
{
  1:string path,
  2:CppModuleMetricsType type,
  3:double value
}

struct CppMetricsModuleAll
{
  1:common.FileId id,
  2:list<CppMetricsModuleSingle> metrics
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
  list<CppMetricsAstNodeSingle> getCppMetricsForAstNode(
    1:common.AstNodeId astNodeId)

  /**
   * This function returns all available C++ metrics
   * for a particular module (file or directory).
   */
  list<CppMetricsModuleSingle> getCppMetricsForModule(
    1:common.FileId fileId)

  /**
   * This function returns all available C++ metrics
   * (AST node-level) for a particular path.
   *
   * The given path is a handled as a prefix.
   */
  map<common.AstNodeId, list<CppMetricsAstNodeSingle>> getCppAstNodeMetricsForPath(
    1:string path)

  /**
   * This function returns all available C++ metrics
   * (file-level) for a particular path.
   *
   * The given path is a handled as a prefix.
   */
  map<common.FileId, list<CppMetricsModuleSingle>> getCppFileMetricsForPath(
    1:string path)

  /**
   * This function returns the names of the AST node-level C++ metrics.
   */
  list<CppAstNodeMetricsTypeName> getCppAstNodeMetricsTypeNames()

  /**
   * This function returns the names of module-level C++ metrics.
   */
  list<CppModuleMetricsTypeName> getCppModuleMetricsTypeNames()
}