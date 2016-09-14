#ifndef CORE_FEATURES_ASTNODE_H
#define CORE_FEATURES_ASTNODE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/fileloc.h>
#include <model/buildlog.h>

namespace cc
{
namespace model
{

typedef unsigned long long HashType;

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
    File = 500,
    Other = 1000
  };

  enum class AstType
  {
    Statement,
    TypeLocation,
    Declaration,
    Definition,
    UnDefinition, // for Macros
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

  typedef uint64_t pktype;

  #pragma db id
  pktype id = 0;

  std::string astValue;

  #pragma db null
  FileLoc location;

  #pragma db null
  std::string mangledName;

  HashType mangledNameHash;

  SymbolType symbolType = SymbolType::Other;

  AstType astType = AstType::Other;
  
  bool visibleInSourceCode = true;

  bool operator<(const CppAstNode& other) const { return id < other.id; }

  static std::string symbolTypeToString(SymbolType symboltype);

  static std::string astTypeToString(AstType asttype);

#ifndef NO_INDICES
  #pragma db index ("location_file_idx") member(location.file)
  #pragma db index ("mangledNameHash_astType_idx") members(mangledNameHash, astType)
  #pragma db index ("astType_symbolType_idx") members(astType, symbolType)
#endif
};

inline bool isTypeLocation(CppAstNode::AstType type_)
{
  return
    type_ == CppAstNode::AstType::TypeLocation      ||
    type_ == CppAstNode::AstType::ParameterTypeLoc  ||
    type_ == CppAstNode::AstType::ReturnTypeLoc     ||
    type_ == CppAstNode::AstType::FieldTypeLoc      ||
    type_ == CppAstNode::AstType::GlobalTypeLoc     ||
    type_ == CppAstNode::AstType::LocalTypeLoc;
}

typedef odb::lazy_shared_ptr<CppAstNode> CppAstNodePtr;
typedef std::unordered_set<CppAstNode::pktype> AstCacheType;


#pragma db view object(CppAstNode)
struct CppAstNodeId
{
  CppAstNode::pktype id;
  
  bool operator<(const CppAstNodeId& other) const { return id < other.id; }
};

#pragma db view object(CppAstNode) object(File = LocFile : CppAstNode::location.file) \
  query ((?) + "GROUP BY" + LocFile::id + "ORDER BY" + LocFile::id)
struct AstCountGroupByFiles
{
  #pragma db column(LocFile::id)
  FileId file;

  #pragma db column("count(" + CppAstNode::id + ")")
  std::size_t count;
};

#pragma db view object(BuildLog) object(File: BuildLog::location.file) \
  query((?) + "AND" + (File::type == cc::model::File::CxxSource || File::type == cc::model::File::CSource))
struct BuildLogFileMessageType
{
  #pragma db column(BuildLog::location.file)
  odb::nullable<FileId> fileId;

  #pragma db column(BuildLog::log.type)
  odb::nullable<BuildLogMessage::MessageType> messageType;
};

#pragma db view object(BuildSource) object(File: BuildSource::file) \
  query((File::type == cc::model::File::CxxSource || File::type == cc::model::File::CSource) + "GROUP BY" + BuildSource::file)
struct CxxBuildSources
{
  #pragma db column(BuildSource::file)
  FileId file;
};

} // model
} // cc

#endif//CORE_FEATURES_ASTNODE_H
