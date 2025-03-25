#ifndef CC_MODEL_CPPTYPEDEPENDENCYMETRICS_H
#define CC_MODEL_CPPTYPEDEPENDENCYMETRICS_H

#include <cstdint>
#include <string>
#include <model/cppentity.h>
#include <model/cpprecord.h>
#include <model/cppastnode.h>
#include <model/file.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppTypeDependencyMetrics
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  std::uint64_t entityHash;

  #pragma db not_null
  std::uint64_t dependencyHash;
};

#pragma db view \
  object(CppTypeDependencyMetrics) \
  object(CppAstNode = EntityAstNode : CppTypeDependencyMetrics::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependencyMetrics::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id)
struct CppTypeDependencyMetricsPathView
{
  #pragma db column(CppTypeDependencyMetrics::entityHash)
  std::size_t entityHash;

  #pragma db column(CppTypeDependencyMetrics::dependencyHash)
  std::size_t dependencyHash;

  #pragma db column(EntityFile::path)
  std::string entityPath;

  #pragma db column(DependencyFile::path)
  std::string dependencyPath;
};

#pragma db view \
  object(CppTypeDependencyMetrics) \
  object(CppAstNode = EntityAstNode : CppTypeDependencyMetrics::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependencyMetrics::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id) \
  query((?), distinct)
struct CppDistinctTypeDependencyMetricsPathView
{
  #pragma db column(CppTypeDependencyMetrics::dependencyHash)
  std::size_t dependencyHash;
};

} // model
} // cc

#endif // CC_MODEL_CPPTYPEDEPENDENCYMETRICS_H
