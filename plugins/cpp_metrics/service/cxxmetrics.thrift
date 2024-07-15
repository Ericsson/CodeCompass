include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.cppmetrics
namespace java cc.service.cppmetrics

enum CppAstNodeMetricsType
{
  ParameterCount = 1,
  McCabeFunction = 2,
  McCabeType = 3,
  BumpyRoad = 4,
  LackOfCohesion = 5,
  LackOfCohesionHS = 6,
  EfferentType = 7
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

struct CppMetricsAstNodeDetailed
{
  1:string path,
  2:string file,
  3:string symbolType,
  4:string astType,
  5:i32 startLine,
  6:i32 endLine,
  7:string astValue,
  8:map<CppAstNodeMetricsType, double> metrics
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
   * (AST node-level) and miscellaneous data for a particular path.
   *
   * The given path is a handled as a prefix.
   */
  map<common.AstNodeId, CppMetricsAstNodeDetailed> getCppAstNodeMetricsDetailedForPath(
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