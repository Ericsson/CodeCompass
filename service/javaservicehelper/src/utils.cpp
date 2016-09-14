// $Id$
// Created by Aron Barath, 2013

#include "utils.h"

#include <string>

#include "util/streamlog.h"

#include "model/file.h"
#include "model/file-odb.hxx"
#include "model/java/javaastnode-odb.hxx"
#include "model/java/javatype-odb.hxx"
#include "model/java/javatype_interfaces-odb.hxx"
#include "model/java/javamember-odb.hxx"

namespace cc
{
namespace service
{
namespace language 
{

std::string getFileloc(const model::JavaAstNode& astNode)
{
  if(!astNode.file) return "";

  auto fileloc = baseName(astNode.file.load()->path);
  fileloc += ":" + std::to_string(astNode.loc_start_line);
  fileloc += ":" + std::to_string(astNode.loc_start_col);

  return fileloc;
}

AstNodeInfo createAstNodeInfo(const model::JavaAstNode& astNode)
{
  return createAstNodeInfo(astNode, astNode);
}

AstNodeInfo createAstNodeInfo(
  const model::JavaAstNode& astNode,
  const model::JavaAstNode& defAstNode)
{
  SLog() << "creating AstNodeInfo for " << astNode.astValue;

  AstNodeInfo ret;

  ret.astNodeId.astNodeId = std::to_string(astNode.id);
  ret.astNodeType = symbolTypeToString(astNode.symbolType);

  if(defAstNode.file)
  {
    ret.range.file.fid = std::to_string(defAstNode.file.object_id());

    defAstNode.file.load()->content.load();
    const auto& content = defAstNode.file->content->content;

    model::Range range(model::Position(defAstNode.loc_start_line, 1), model::Position(defAstNode.loc_end_line, std::string::npos));

    const auto ROW_SIZE = 100;

    ret.astNodeSrcText = textRange(content, range, ROW_SIZE);
  }

  ret.astNodeValue = astNode.astValue;

  ret.range.range.startpos.line = defAstNode.loc_start_line;
  ret.range.range.startpos.column = defAstNode.loc_start_col;

  ret.range.range.endpos.line = defAstNode.loc_end_line;
  ret.range.range.endpos.column = defAstNode.loc_end_col;

  ret.documentation = "Documentation";

  SLog() << "created AstNodeInfo for " << astNode.astValue;

  return ret;
}

std::vector<model::JavaType> getInheritsFromTypes(std::shared_ptr<odb::database> db,
  JavaOdbQuery & query, const model::JavaAstNode& astNode)
{
  return getInheritsFromTypes(db, query, astNode.mangledNameHash);
}

std::vector<model::JavaType> getInheritsFromTypes(std::shared_ptr<odb::database> db,
  JavaOdbQuery & query, JavaHashType mangledNameHash)
{
  std::vector<model::JavaType> ret;
  auto javaType = query.queryEntityByHash<model::JavaType>(mangledNameHash);

  if(javaType.superClass.load())
  {
    ret.push_back(*javaType.superClass);
  }

  typedef odb::query<model::JavaType_Interfaces> QTypeIf;

  for(model::JavaType_Interfaces & mapping : db->query<model::JavaType_Interfaces>(QTypeIf::type == javaType.mangledNameHash))
  {
    try
    {
      ret.push_back(*mapping.iface.load());
    }
    catch(const std::exception& ex)
    {
      SLog(util::ERROR)<< "Exception caught: " << ex.what();
    }
  }

  return ret;
}

std::vector<model::JavaType> getInheritsByTypes(std::shared_ptr<odb::database> db,
  JavaOdbQuery & query, const model::JavaAstNode& astNode, bool includeGenInst)
{
  return getInheritsByTypes(db, query, astNode.mangledNameHash, includeGenInst);
}

std::vector<model::JavaType> getInheritsByTypes(std::shared_ptr<odb::database> db,
  JavaOdbQuery & query, JavaHashType mangledNameHash, bool includeGenInst)
{
  std::vector<model::JavaType> ret;
  model::JavaType javaType = query.queryEntityByHash<model::JavaType>(mangledNameHash);

  typedef odb::query<model::JavaType> QType;
  typedef odb::query<model::JavaType_Interfaces> QTypeIf;

  std::vector<model::JavaType::pktype> type_ids;
  type_ids.push_back(javaType.mangledNameHash);
  if(includeGenInst)
  {
    for(const model::JavaType & inst : db->query<model::JavaType>(
      QType::genericImpl==javaType.mangledNameHash))
    {
      type_ids.push_back(inst.mangledNameHash);
    }
  }

  for(model::JavaType & type : db->query<model::JavaType>(QType::superClass.in_range(type_ids.begin(), type_ids.end())))
  {
    ret.push_back(type);
  }

  for(model::JavaType_Interfaces & mapping : db->query<model::JavaType_Interfaces>(QTypeIf::iface.in_range(type_ids.begin(), type_ids.end())))
  {
    try
    {
      ret.push_back(*mapping.type.load());
    }
    catch(const std::exception& ex)
    {
      SLog(util::ERROR)<< "Exception caught: " << ex.what();
    }
  }

  return ret;
}

std::vector<JavaHashType> getOverrideHashes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  const model::JavaFunction & function)
{
  std::vector<JavaHashType> hashes = { function.mangledNameHash };

  if(function.type.load() && function.type.load()->astNodePtr.load())
  {
    for(model::JavaType & type : getInheritsFromTypes(db, query, *function.type.load()->astNodePtr.load()))
    {
      if(type.astNodePtr)
      {
        for(odb::lazy_weak_ptr<cc::model::JavaFunction> & func : type.functions)
        {
          if(func.load()->signature == function.signature)
          {
            hashes.push_back(func.load()->mangledNameHash);
          }
        }
      }
    }
  }

  return hashes;
}

} // language
} // service
} // cc

