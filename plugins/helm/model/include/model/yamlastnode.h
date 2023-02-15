#ifndef CC_MODEL_YAMLASTNODE_H
#define CC_MODEL_YAMLASTNODE_H

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

typedef std::uint64_t YamlAstNodeId;

#pragma db object
struct YamlAstNode
{
  enum class SymbolType
  {
      Key,
      Value,
      NestedKey,
      NestedValue,
      Other
  };

  enum class AstType
  {
      NULLTYPE,
      SCALAR,
      MAP,
      SEQUENCE,
      UNDEFINED = 50
  };

  #pragma db id
  YamlAstNodeId id = 0;

  std::string astValue;

  #pragma db null
  FileLoc location;

  std::uint64_t entityHash;

  SymbolType symbolType = SymbolType::Other;

  AstType astType = AstType::NULLTYPE;

  std::string toString() const;

  bool operator< (const YamlAstNode& other) const { return id <  other.id; }
  bool operator==(const YamlAstNode& other) const { return id == other.id; }
};

typedef std::shared_ptr<YamlAstNode> YamlAstNodePtr;

inline std::string symbolTypeToString(YamlAstNode::SymbolType type_)
{
  switch (type_)
  {
    case YamlAstNode::SymbolType::Key: return "Key";
    case YamlAstNode::SymbolType::Value: return "Value";
    case YamlAstNode::SymbolType::NestedKey: return "NestedKey";
    case YamlAstNode::SymbolType::NestedValue: return "NestedValue";
    case YamlAstNode::SymbolType::Other: return "Other";
  }

  return std::string();
}

inline std::string astTypeToString(YamlAstNode::AstType type_)
{
  switch (type_)
  {
    case YamlAstNode::AstType::NULLTYPE: return "Null";
    case YamlAstNode::AstType::SCALAR: return "Scalar";
    case YamlAstNode::AstType::MAP: return "Map";
    case YamlAstNode::AstType::SEQUENCE: return "Sequence";
    case YamlAstNode::AstType::UNDEFINED: return "Undefined";
  }

  return std::string();
}

inline std::string YamlAstNode::toString() const
{
  return std::string("YamlAstNode")
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

inline std::uint64_t createIdentifier(const YamlAstNode& astNode_)
{
  using SymbolTypeInt
          = std::underlying_type<model::YamlAstNode::SymbolType>::type;
  using AstTypeInt
          = std::underlying_type<model::YamlAstNode::AstType>::type;

  std::string res;

  res
    .append(astNode_.astValue).append(":")
    .append(std::to_string(astNode_.entityHash)).append(":")
    .append(std::to_string(
      static_cast<SymbolTypeInt>(astNode_.symbolType))).append(":")
    .append(std::to_string(
      static_cast<AstTypeInt>(astNode_.astType))).append(":");

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

}
}

#endif // CC_MODEL_YAMLASTNODE_H
