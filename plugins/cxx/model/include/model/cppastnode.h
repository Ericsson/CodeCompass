#ifndef CC_MODEL_CXXASTNODE_H
#define CC_MODEL_CXXASTNODE_H

#include <cstdint>
#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/file.h>
#include <model/fileloc.h>

#include <util/hash.h>

namespace cc
{
namespace model
{

typedef std::uint64_t CppAstNodeId;

#pragma db object
struct CppAstNode
{
  enum class SymbolType
  {
    Variable,
    Function,
    FunctionPtr,
    Type,
    Typedef,
    Macro,
    Enum,
    EnumConstant,
    Namespace,
    StringLiteral,
    File = 500, // TODO: Is this for #include?
    Other = 1000
  };

  enum class AstType
  {
    Statement,
    TypeLocation,
    Declaration,
    Definition,
    UnDefinition,
    Usage,
    Read,
    Write,
    VirtualCall,
    ParameterTypeLoc,
    ReturnTypeLoc,
    FieldTypeLoc,
    GlobalTypeLoc,
    LocalTypeLoc,
    Other = 1000
  };

  virtual ~CppAstNode() {}

  friend class odb::access;

  #pragma db id
  CppAstNodeId id = 0;

  std::string astValue;

  #pragma db null
  FileLoc location;

  #pragma db null
  std::string mangledName;

  std::uint64_t mangledNameHash;

  SymbolType symbolType = SymbolType::Other;

  AstType astType = AstType::Other;

  bool visibleInSourceCode = true;

  bool operator<(const CppAstNode& other) const { return id < other.id; }

  static std::string symbolTypeToString(SymbolType symbolType);
  static std::string astTypeToString(AstType astType);

#ifndef NO_INDICES
  #pragma db index("location_file_idx") member(location.file)
  #pragma db index("mangledNameHash_astType_idx") members(mangledNameHash, astType)
  #pragma db index("astType_symbolType_idx") members(astType, symbolType)
#endif
};

typedef std::shared_ptr<CppAstNode> CppAstNodePtr;

inline bool isTypeLocation(CppAstNode::AstType type_)
{
  return
    type_ == CppAstNode::AstType::TypeLocation     ||
    type_ == CppAstNode::AstType::ParameterTypeLoc ||
    type_ == CppAstNode::AstType::ReturnTypeLoc    ||
    type_ == CppAstNode::AstType::FieldTypeLoc     ||
    type_ == CppAstNode::AstType::GlobalTypeLoc    ||
    type_ == CppAstNode::AstType::LocalTypeLoc;
}

inline std::uint64_t createIdentifier(const CppAstNode& astNode_)
{
  using SymbolTypeInt
    = std::underlying_type<model::CppAstNode::SymbolType>::type;
  using AstTypeInt
    = std::underlying_type<model::CppAstNode::AstType>::type;

  std::string res;

  // For string concatenation we use append() function because this is faster
  // than cancatenating strings with operator+(). operator+=() would be more
  // readable but unfortunately that is right associative. This function is
  // invoked many times, so it is important to create identifier as fast as
  // possible.

  res
    .append(astNode_.astValue).append(":")
    .append(astNode_.mangledName).append(":")
    .append(std::to_string(
      static_cast<SymbolTypeInt>(astNode_.symbolType))).append(":")
    .append(std::to_string(
      static_cast<AstTypeInt>(astNode_.astType))).append(":")
    .append(std::to_string(astNode_.visibleInSourceCode)).append(":");

  if (astNode_.location.file)
    res
      .append(std::to_string(
        astNode_.location.file->id)).append(":")
      .append(std::to_string(
        astNode_.location.range.start.line)).append(":")
      .append(std::to_string(
        astNode_.location.range.start.column)).append(":")
      .append(std::to_string(
        astNode_.location.range.end.line)).append(":")
      .append(std::to_string(
        astNode_.location.range.end.column)).append(":");
  else
    res.append("null");

  return util::fnvHash(res);
}

#pragma db view object(CppAstNode)
struct CppAstNodeIds
{
  CppAstNodeId id;
};

// TODO: Is the ordering needed?
#pragma db view \
  object(CppAstNode) object(File = LocFile : CppAstNode::location.file) \
  query ((?) + "GROUP BY" + LocFile::id + "ORDER BY" + LocFile::id)
struct AstCountGroupByFiles
{
  #pragma db column(LocFile::id)
  FileId file;

  #pragma db column("count(" + CppAstNode::id + ")")
  std::size_t count;
};

}
}

#endif
