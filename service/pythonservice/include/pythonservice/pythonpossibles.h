#ifndef SERVICE_PYTHONSERVICE_PYTHONPOSSIBLES_H
#define SERVICE_PYTHONSERVICE_PYTHONPOSSIBLES_H

#include <memory>

#include <model/python/pythonastnode.h>
#include <pythonservice/pythonqueryhelper.h>

namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{


class PythonPossibleHandler
{
protected:
  PythonPossibleHandler(std::shared_ptr<odb::database> db_)
    : db(db_)
    , helper(PythonQueryHelper(db_))
  {}

public:
  virtual ~PythonPossibleHandler()
  {}

  static std::vector<model::PythonAstNode> getPossibleDefs(
    const model::PythonAstNode& astNode_,
    std::shared_ptr<odb::database> db_,
    const bool isLimited = false);

  static std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_,
    std::shared_ptr<odb::database> db_);

protected:
  virtual std::vector<model::PythonAstNode> getPossibleDefs(
    const model::PythonAstNode& astNode_,
    const bool isLimited = false) const
  {
    return {};
  }

  virtual std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_) const
  {
    return {};
  }

protected:
  std::shared_ptr<odb::database> db;
  PythonQueryHelper helper;
};

} // python
} // language
} // service
} // cc

#endif // SERVICE_PYTHONSERVICE_PYTHONPOSSIBLES_H
