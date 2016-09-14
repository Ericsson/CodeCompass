#ifndef SERVICE_PYTHONSERVICE_TREEHANDLER_H
#define SERVICE_PYTHONSERVICE_TREEHANDLER_H

#include <memory>
#include <string>

#include <pythonservice/pythonqueryhelper.h>

#include "language-api/language_types.h"

namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{


class TreeHandler
{
public:
  static std::unique_ptr<TreeHandler> getHandler(
    std::shared_ptr<odb::database> db, model::PythonAstNode::AstType astType_);

  TreeHandler(std::shared_ptr<odb::database> db)
    : db(db), helper(db)
  { }

  virtual ~TreeHandler()
  { }

  virtual std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const = 0;

  virtual std::vector<InfoNode> getSubInfoTree(
    const model::PythonAstNode& astNode_,
    const InfoQuery& query) const = 0;

protected:
  std::shared_ptr<odb::database> db;
  PythonQueryHelper helper;
};



class FileTreeHandler
{
public:
  static std::unique_ptr<FileTreeHandler> getHandler(
    std::shared_ptr<odb::database> db, model::PythonAstNode::AstType astType_);

  FileTreeHandler(std::shared_ptr<odb::database> db)
    : db(db), helper(db)
  { }

  virtual ~FileTreeHandler()
  { }

  std::vector<InfoNode> getInfoTreeForFile(
    const model::FileId& fid);

  std::vector<InfoNode> getSubInfoTreeForFile(
    const model::FileId& fid,
    const InfoQuery& infoQuery);

private:
  std::shared_ptr<odb::database> db;
  PythonQueryHelper helper;

  enum class SubQuery
  {
    Classes = 1,
    Functions,
    Globals
  };
};

template<typename Entity>
std::string getFileLoc(const Entity& entity)
{
  return
    std::to_string(entity.location.range.start.line) +
    ":" + std::to_string(entity.location.range.start.column);
}

inline std::string getFileLocByAstNode(const AstNodeInfo& entity)
{
  return std::to_string(entity.range.range.startpos.line) +
    ":" + std::to_string(entity.range.range.startpos.column);
}

InfoNode makeInfoNode(
  const std::vector<std::string>& category,
  const std::string& label_,
  const std::string& value_,
  const AstNodeInfo& astValue_ = {});

InfoNode makeInfoQueryNode(
  const std::vector<std::string>& category,
  const int queryId,
  const std::vector<std::string>& filters = {});

void sortNodeInfoByFilePos(
  std::vector<InfoNode>& ret,
  std::shared_ptr<odb::database> db_);

template<typename AstNodeIterableCollection>
std::vector<InfoNode> sortNodeInfoByPackagePos(
  std::vector<InfoNode>& ret,
  AstNodeIterableCollection& collection,
  const std::vector<std::string> category_,
  const InfoQuery& infoQuery,
  const PythonQueryHelper& helper);

struct compInfoNodeByPos
{
  bool operator()(
    const InfoNode& lhs,
    const InfoNode& rhs)
  {
    const auto& lhs_line   = lhs.astValue.range.range.startpos.line;
    const auto& lhs_column = lhs.astValue.range.range.startpos.column;
    const auto& rhs_line   = rhs.astValue.range.range.startpos.line;
    const auto& rhs_column = rhs.astValue.range.range.startpos.column;

    return
      lhs.category.size() < rhs.category.size()
      ||
      (
        lhs.category.size() == rhs.category.size()
        &&
        (
          lhs_line < rhs_line
          ||
          (
            lhs_line == rhs_line
            &&
            lhs_column < rhs_column
          )
        )
      );
  }
};

} // python
} // language
} // service
} // cc

#endif // SERVICE_PYTHONSERVICE_TREEHANDLER_H