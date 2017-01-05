// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAASTNODE_H
#define MODEL_JAVA_JAVAASTNODE_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/asttype.h>
#include <model/fileloc.h>

namespace cc
{
namespace model
{

#pragma db object
struct JavaAstNode
{
  JavaAstNode() {}
  virtual ~JavaAstNode() {}

  friend class odb::access;

  typedef int64_t pktype;

  #pragma db id
  pktype id;

  odb::lazy_shared_ptr<File> file;

  #pragma db null
  std::string astValue;

  int loc_start_line;
  int loc_start_col;
  int loc_end_line;
  int loc_end_col;

  #pragma db null
  std::string mangledName;

  long long mangledNameHash;

  #pragma db null
  SymbolType symbolType;

  #pragma db null
  AstType astType;

  bool operator<(const JavaAstNode& other) const { return id < other.id; }
};

typedef odb::lazy_shared_ptr<JavaAstNode> JavaAstNodePtr;

#pragma db view object(JavaAstNode)
struct JavaAstNodeId
{
  JavaAstNode::pktype id;

  bool operator<(const JavaAstNodeId& other) const { return id < other.id; }
};

#pragma db view object(JavaAstNode) object(File) \
  query ((?) + "GROUP BY" + File::id + "ORDER BY" + File::id)
struct AstCountGroupByFilesJava
{
  #pragma db column(File::id)
  FileId file;

  #pragma db column("count(" + JavaAstNode::id + ")")
  std::size_t count;
};

} // model
} // cc

#endif // MODEL_JAVA_JAVAASTNODE_H
