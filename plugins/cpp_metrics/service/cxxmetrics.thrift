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

service CppMetricsService
{
  //double getCppMetricsForAstNode(
    //1:common.AstNodeId astNodeId,
    //2:CppMetricsType type)
}