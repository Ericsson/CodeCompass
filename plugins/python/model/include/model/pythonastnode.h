#ifndef CC_MODEL_PYTHONASTNODE_H
#define CC_MODEL_PYTHONASTNODE_H

#include <string>

#include <odb/nullable.hxx>
#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/fileloc.h>

#include <util/hash.h>

namespace cc
{
namespace model
{

typedef std::uint64_t PythonAstNodeId;

#pragma db object
struct PythonAstNode
{
    enum class SymbolType
    {
        Variable,
        Function,
        Class,
        Module,
        Other
    };

    enum class AstType
    {
        Declaration,
        Usage,
        Other
    };

    virtual ~PythonAstNode() {}

    #pragma db id
    PythonAstNodeId id = 0;

    std::string astValue;

    std::string qualifiedName;

    #pragma db null
    FileLoc location;

    SymbolType symbolType = SymbolType::Other;

    AstType astType = AstType::Other;

    std::string toString() const;

    bool operator< (const PythonAstNode& other) const { return id <  other.id; }
    bool operator==(const PythonAstNode& other) const { return id == other.id; }
};

typedef std::shared_ptr<PythonAstNode> PythonAstNodePtr;

inline std::string symbolTypeToString(PythonAstNode::SymbolType type_)
{
    switch (type_)
    {
        case PythonAstNode::SymbolType::Variable: return "Variable";
        case PythonAstNode::SymbolType::Function: return "Function";
        case PythonAstNode::SymbolType::Class: return "Class";
        case PythonAstNode::SymbolType::Module: return "Module";
        case PythonAstNode::SymbolType::Other: return "Other";
    }

    return std::string();
}

inline std::string astTypeToString(PythonAstNode::AstType type)
{
    switch (type) {
        case PythonAstNode::AstType::Usage: return "Usage";
        case PythonAstNode::AstType::Declaration: return "Declaration";
        case PythonAstNode::AstType::Other: return "Other";
    }

    return std::string();
}

inline std::string PythonAstNode::toString() const
{
    return std::string("PythonAstNode")
        .append("\nid = ").append(std::to_string(id))
        .append("\nastValue = ").append(astValue)
        .append("\nqualifiedName = ").append(qualifiedName)
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

inline std::uint64_t createIdentifier(const PythonAstNode& astNode_)
{
    using SymbolTypeInt
    = std::underlying_type<model::PythonAstNode::SymbolType>::type;
    using AstTypeInt
    = std::underlying_type<model::PythonAstNode::AstType>::type;

    std::string res;

    res
        .append(astNode_.astValue).append(":")
        .append(std::to_string(astNode_.qualifiedName)).append(":")
        .append(std::to_string(static_cast<SymbolTypeInt>(astNode_.symbolType))).append(":")
        .append(std::to_string(static_cast<AstTypeInt>(astNode_.astType))).append(":");

    if (astNode_.location.file != nullptr){
        res
            .append(std::to_string(astNode_.location.file->id)).append(":")
            .append(std::to_string(astNode_.location.file->range.start.line)).append(":")
            .append(std::to_string(astNode_.location.file->range.start.column)).append(":")
            .append(std::to_string(astNode_.location.file->range.end.line)).append(":")
            .append(std::to_string(astNode_.location.file->range.end.column)).append(":");
    } else {
        res.append("null");
    }

    return util::fnvHash(res);
}

#pragma db view \
  object(PythonAstNode) object(File = LocFile : PythonAstNode::location.file) \
  query ((?) + "GROUP BY" + LocFile::id + "ORDER BY" + LocFile::id)
struct PythonAstCountGroupByFiles
{
    #pragma db column(LocFile::id)
    FileId file;

    #pragma db column("count(" + PythonAstNode::id + ")")
    std::size_t count;
};

#pragma db view object(PythonAstNode)
struct PythonAstCount
{
    #pragma db column("count(" + PythonAstNode::id + ")")
    std::size_t count;
};

}
}

#endif
