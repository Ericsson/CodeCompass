#ifndef CC_MODEL_CPPTYPEDEPENDENCY_H
#define CC_MODEL_CPPTYPEDEPENDENCY_H

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
struct CppTypeDependency
{
  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  std::uint64_t entityHash;

  #pragma db not_null
  std::uint64_t dependencyHash;

  std::string toString() const
  {
    return std::string("CppTypeDependency")
      .append("\nid = ").append(std::to_string(id))
      .append("\nentityHash = ").append(std::to_string(entityHash))
      .append("\ndependencyHash = ").append(std::to_string(dependencyHash));
  }

#pragma db index member(entityHash)
#pragma db index member(dependencyHash)
};

typedef std::shared_ptr<CppTypeDependency> CppTypeDependencyPtr;

#pragma db view \
  object(CppTypeDependency) \
  object(CppAstNode = EntityAstNode : CppTypeDependency::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependency::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id)
struct CppTypeDependencyPathView
{
  #pragma db column(CppTypeDependency::entityHash)
  std::size_t entityHash;

  #pragma db column(CppTypeDependency::dependencyHash)
  std::size_t dependencyHash;

  #pragma db column(EntityFile::path)
  std::string entityPath;

  #pragma db column(DependencyFile::path)
  std::string dependencyPath;
};

#pragma db view \
  object(CppTypeDependency) \
  object(CppAstNode = EntityAstNode : CppTypeDependency::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependency::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id)
struct CppTypeDependency_Count
{
  #pragma db column("count(" + CppTypeDependency::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppTypeDependency) \
  object(CppAstNode = EntityAstNode : CppTypeDependency::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependency::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id)
struct CppTypeDependency_Distinct_D_Count
{
  #pragma db column("count(distinct" + CppTypeDependency::dependencyHash + ")")
  std::size_t count;
};

#pragma db view \
  object(CppTypeDependency) \
  object(CppAstNode = EntityAstNode : CppTypeDependency::entityHash == EntityAstNode::entityHash \
    && EntityAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = EntityFile : EntityAstNode::location.file == EntityFile::id) \
  object(CppAstNode = DependencyAstNode : CppTypeDependency::dependencyHash == DependencyAstNode::entityHash \
    && DependencyAstNode::astType == cc::model::CppAstNode::AstType::Definition) \
  object(File = DependencyFile : DependencyAstNode::location.file == DependencyFile::id)
struct CppTypeDependency_Distinct_E_Count
{
  #pragma db column("count(distinct" + CppTypeDependency::entityHash + ")")
  std::size_t count;
};

#pragma db view \
  object(CppTypeDependency)
struct CppTypeDependency_Efferent_Count
{
  #pragma db column("count(distinct" + CppTypeDependency::dependencyHash + ")")
  std::size_t count;
};

#pragma db view \
  object(CppTypeDependency)
struct CppTypeDependency_Afferent_Count
{
  #pragma db column("count(distinct" + CppTypeDependency::entityHash + ")")
  std::size_t count;
};

} // model
} // cc

#endif // CC_MODEL_CPPTYPEDEPENDENCY_H
