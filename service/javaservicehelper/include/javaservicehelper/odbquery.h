// $Id$
// Created by Aron Barath, 2013

#ifndef JAVASERVICEHELPER_ODBQUERY_H
#define JAVASERVICEHELPER_ODBQUERY_H

#include "langservicelib/odbquery.h"

#include "model/position.h"
#include "model/java/javaastnode.h"
#include "model/java/javaastnode-odb.hxx"
#include "model/java/javatype.h"
#include "model/java/javatype-odb.hxx"
#include "model/java/javaenumconstant.h"
#include "model/java/javaenumconstant-odb.hxx"
#include "model/java/javafunction.h"
#include "model/java/javafunction-odb.hxx"
#include "model/java/javavariable.h"
#include "model/java/javavariable-odb.hxx"

namespace cc
{
namespace service
{
namespace language 
{

typedef unsigned long long JavaHashType;

template<>
template<>
inline model::FileId
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 getFileId(const model::JavaAstNode & node)
{
  return node.file.object_id();
}

template<>
template<>
inline model::Range
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 getRange(const model::JavaAstNode & node)
{
  return model::Range(
    model::Position(node.loc_start_line, node.loc_start_col),
    model::Position(node.loc_end_line, node.loc_end_col));
}

template<>
template<>
inline typename OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::AstQuery
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 astFileId<model::JavaAstNode>(const model::FileId fileId)
{
  return {AstQuery::file == fileId};
}

template<>
template<>
inline typename OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::AstQuery
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 astInRange<model::JavaAstNode>(const model::Range range)
{
  const auto &start = range.start;
  const auto &end = range.end;

  return {
    // start <= pos
    ((AstQuery::loc_start_line == start.line
        && start.column <= AstQuery::loc_start_col)
      || start.line < AstQuery::loc_start_line)
    &&
    // pos < end
    ((AstQuery::loc_end_line == end.line
        && AstQuery::loc_end_col <= end.column)
      || AstQuery::loc_end_line < end.line)};
}

template<>
template<>
inline typename OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::AstQuery
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 astContains<model::JavaAstNode>(const model::Position position)
{
  const auto line = position.line;
  const auto column = position.column;

  return {
      // StartPos <= Pos
      ((AstQuery::loc_start_line == line
          && AstQuery::loc_start_col <= column)
        || AstQuery::loc_start_line < line)
      &&
      // Pos < EndPos
      ((AstQuery::loc_end_line == line
          && column <= AstQuery::loc_end_col)
        || line < AstQuery::loc_end_line)
  };
}

template<>
template<>
inline std::string
 OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>::
 getQualifiedName<model::JavaAstNode>(const model::JavaAstNode& astNode) const
{
  using namespace model;

  switch(astNode.symbolType)
  {
    case SymbolType::Function:
      return queryEntityByHash<JavaFunction>(astNode.mangledNameHash).qualifiedName;

    case SymbolType::Variable:
      return queryEntityByHash<JavaVariable>(astNode.mangledNameHash).qualifiedName;

    case SymbolType::Type:
      return queryEntityByHash<JavaType>(astNode.mangledNameHash).qualifiedName;

    case SymbolType::Enum: // Enum is represented with a special type
      return queryEntityByHash<JavaType>(astNode.mangledNameHash).qualifiedName;

    case SymbolType::EnumConstant:
      return queryEntityByHash<JavaEnumConstant>(astNode.mangledNameHash).qualifiedName;

    default: break;
  }

  return {};
}

typedef OdbQuery<model::JavaAstNode, JavaHashType, model::JavaType, model::JavaEnum>
 JavaOdbQuery;

} // language
} // service
} // cc

#endif // JAVASERVICEHELPER_ODBQUERY_H

