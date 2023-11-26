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
    NamespaceAlias,
    StringLiteral,
    File = 500,
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
    TypedefTypeLoc,
    InheritanceTypeLoc,
    UsingLoc,
    Other = 1000
  };

  virtual ~CppAstNode() {}

  #pragma db id
  CppAstNodeId id = 0;

  std::string astValue;

  #pragma db null
  FileLoc location;

  std::uint64_t entityHash;

  SymbolType symbolType = SymbolType::Other;

  AstType astType = AstType::Other;

  bool visibleInSourceCode = true;

  std::string toString() const;

  bool operator< (const CppAstNode& other) const { return id <  other.id; }
  bool operator==(const CppAstNode& other) const { return id == other.id; }

#pragma db index("location_file_idx") member(location.file)
#pragma db index("entityHash_astType_idx") members(entityHash, astType)
#pragma db index("astType_symbolType_idx") members(astType, symbolType)
};

typedef std::shared_ptr<CppAstNode> CppAstNodePtr;

inline std::string symbolTypeToString(CppAstNode::SymbolType type_)
{
  switch (type_)
  {
    case CppAstNode::SymbolType::Variable: return "Variable";
    case CppAstNode::SymbolType::Function: return "Function";
    case CppAstNode::SymbolType::FunctionPtr: return "FunctionPtr";
    case CppAstNode::SymbolType::Type: return "Type";
    case CppAstNode::SymbolType::Typedef: return "Typedef";
    case CppAstNode::SymbolType::Macro: return "Macro";
    case CppAstNode::SymbolType::Enum: return "Enum";
    case CppAstNode::SymbolType::EnumConstant: return "EnumConstant";
    case CppAstNode::SymbolType::Namespace: return "Namespace";
    case CppAstNode::SymbolType::NamespaceAlias: return "NamespaceAlias";
    case CppAstNode::SymbolType::StringLiteral: return "StringLiteral";
    case CppAstNode::SymbolType::File: return "File";
    case CppAstNode::SymbolType::Other: return "Other";
  }

  return std::string();
}

inline std::string astTypeToString(CppAstNode::AstType type_)
{
  switch (type_)
  {
    case CppAstNode::AstType::Statement: return "Statement";
    case CppAstNode::AstType::TypeLocation: return "TypeLocation";
    case CppAstNode::AstType::Declaration: return "Declaration";
    case CppAstNode::AstType::Definition: return "Definition";
    case CppAstNode::AstType::UnDefinition: return "UnDefinition";
    case CppAstNode::AstType::Usage: return "Usage";
    case CppAstNode::AstType::Read: return "Read";
    case CppAstNode::AstType::Write: return "Write";
    case CppAstNode::AstType::VirtualCall: return "VirtualCall";
    case CppAstNode::AstType::ParameterTypeLoc: return "ParameterTypeLoc";
    case CppAstNode::AstType::ReturnTypeLoc: return "ReturnTypeLoc";
    case CppAstNode::AstType::FieldTypeLoc: return "FieldTypeLoc";
    case CppAstNode::AstType::GlobalTypeLoc: return "GlobalTypeLoc";
    case CppAstNode::AstType::LocalTypeLoc: return "LocalTypeLoc";
    case CppAstNode::AstType::TypedefTypeLoc: return "TypedefTypeLoc";
    case CppAstNode::AstType::InheritanceTypeLoc: return "InheritanceTypeLoc";
    case CppAstNode::AstType::UsingLoc: return "UsingLoc";
    case CppAstNode::AstType::Other: return "Other";
  }

  return std::string();
}

inline std::string CppAstNode::toString() const
{
  return std::string("CppAstNode")
    .append("\nid = ").append(std::to_string(id))
    .append("\nastValue = ").append(astValue)
    .append("\nlocation = ").append(location.file->path).append(" (")
    .append(std::to_string(
      static_cast<signed>(location.range.start.line))).append(":")
    .append(std::to_string(
      static_cast<signed>(location.range.start.column))).append(" - ")
    .append(std::to_string(
      static_cast<signed>(location.range.end.line))).append(":")
    .append(std::to_string(
      static_cast<signed>(location.range.end.column))).append(")")
    .append("\nsymbolType = ").append(symbolTypeToString(symbolType))
    .append("\nastType = ").append(astTypeToString(astType));
}

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
  // than concatenating strings with operator+(). operator+=() would be more
  // readable but unfortunately that is right associative. This function is
  // invoked many times, so it is important to create identifier as fast as
  // possible.

  res
    .append(astNode_.astValue).append(":")
    .append(std::to_string(astNode_.entityHash)).append(":")
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

#pragma db view object(CppAstNode)
struct CppAstCount
{
  #pragma db column("count(" + CppAstNode::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppAstNode) \
  object(File : CppAstNode::location.file) \
  query((CppAstNode::astType == cc::model::CppAstNode::AstType::Read \
      || CppAstNode::astType == cc::model::CppAstNode::AstType::Write) && (?))
struct CppRWAstNodeWithHashAndLoc
{
  typedef cc::model::Position::PosType PosType;

  #pragma db column(CppAstNode::entityHash)
  std::uint64_t entityHash;

  #pragma db column(CppAstNode::location.range.start.line)
  PosType startLine;
  #pragma db column(CppAstNode::location.range.start.column)
  PosType startColumn;
  #pragma db column(CppAstNode::location.range.end.line)
  PosType endLine;
  #pragma db column(CppAstNode::location.range.end.column)
  PosType endColumn;

  #pragma db column(File::path)
  std::string filePath;
};
}
}

#endif
