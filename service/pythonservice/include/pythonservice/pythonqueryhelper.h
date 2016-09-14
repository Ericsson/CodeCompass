#ifndef SERVICE_PYTHONSERVICE_PYTHONQUERYHELPER_H
#define SERVICE_PYTHONSERVICE_PYTHONQUERYHELPER_H

#include <vector>
#include <set>
#include <memory>
#include <string>

#include <odb/database.hxx>

#include <odb/query.hxx>
#include <odb/query-dynamic.hxx>
#include <odb/result.hxx>

#include "langservicelib/utils.h"
#include "language-api/LanguageService.h"

#include <model/fileloc.h>
#include <util/streamlog.h>
#include <model/python/pythonastnode-odb.hxx>
#include <model/python/pythonbinding-odb.hxx>
#include <model/python/pythonastnode-odb.hxx>
#include <model/python/pythonattribute-odb.hxx>
#include <model/python/pythonbinding-odb.hxx>
#include <model/python/pythonclassdef-odb.hxx>
#include <model/python/pythondecorator-odb.hxx>
#include <model/python/pythonfunctiondef-odb.hxx>
#include <model/python/pythonfunctioncall-odb.hxx>
#include <model/python/pythonfunctionparam-odb.hxx>
#include <model/python/pythoninheritance-odb.hxx>
#include <model/python/pythonreference-odb.hxx>
#include <model/python/pythonunknown-odb.hxx>
#include <model/python/pythonvariable-odb.hxx>
#include <model/python/pythonvariableref-odb.hxx>
#include <model/python/pythonviews-odb.hxx>

namespace cc
{
namespace service
{ 
namespace language
{
namespace python
{

typedef odb::query<model::PythonAstNode> queryAstNode;
typedef odb::query<model::PythonAttribute> queryAttribute;
typedef odb::query<model::PythonBinding> queryBinding;
typedef odb::query<model::PythonClassDef> queryClassDef;
typedef odb::query<model::PythonDecorator> queryDecorator;
typedef odb::query<model::PythonFunctionDef> queryFunctionDef;
typedef odb::query<model::PythonFunctionParam> queryParameter;
typedef odb::query<model::PythonInheritance> queryInheritance;
typedef odb::query<model::PythonReference> queryReference;
typedef odb::query<model::PythonUnknown> queryUnknown;
typedef odb::query<model::PythonVariable> queryVariable;
typedef odb::query<model::PythonVariableRef> queryVariableRef;
typedef odb::query<model::PythonFunctionCall> queryFunctionCall;


class PythonQueryHelper
{
public:
  PythonQueryHelper(std::shared_ptr<odb::database> db)
  : db(db)
  {}

public:
  static const std::set<std::string> common_python_keywords;

  /**
   * Get ast node at the given position.
   * @param position
   * @return nested query predicate
   */
  model::PythonAstNode queryAstNodeByPosition(
    const ::cc::service::core::FilePosition & fpos_) const;

  /**
   * Get the corresponding bindings for a ast node no matter
   * those are the referer or base binding of the node.
   * @param ast node
   * @return bindings
   */
  std::vector<model::PythonBinding> queryBindingByAstNode(
    const model::PythonAstNode & astNode_) const;

  /**
   * Get the corresponding bindings for a ast node no matter
   * those are the referer or base binding of the node.
   * @param ast node id
   * @return bindings
   */
  std::vector<model::PythonBinding> queryBindingByAstNode(
    const model::PythonAstNode::pktype & astNodeId_) const;

  /**
   * Get usage of the given ast node.
   * @param ast node
   * @return bunch of nodes
   */
  std::vector<model::PythonAstNode> queryUsageByAstNode(
    const model::PythonAstNode & astNode_) const;

  /**
   * Get possible definitions of an ast node
   * @param ast node id
   * @param resolved definitions
   * @return bunch of possible definitions
   */
  std::vector<model::PythonAstNode> queryPossibleDefs(
    const model::PythonAstNode::pktype & astNodeId_,
    const std::vector<model::PythonBinding> & resolvedDefs_ = {},
    const bool isLimited = false) const;

  /**
   * Get possible definitions of an ast node
   * @param ast node
   * @param resolved definitions
   * @return bunch of possible definitions
   */
  std::vector<model::PythonAstNode> queryPossibleDefs(
    const model::PythonAstNode & astNode_,
    const std::vector<model::PythonBinding> & resolvedDefs_ = {},
    const bool isLimited = false) const;

  /**
   * Get possible usages of an ast node
   * @param ast node
   * @paran resikved usages
   * @return bunch of possible definitions
   */
  std::vector<model::PythonAstNode> queryPossibleUsages(
    const model::PythonAstNode::pktype & astNodeId_,
    const std::vector<model::PythonBinding> & resolvedUsages_ = {}) const;

  /**
   * Get possible usages of an ast node
   * @param ast node
   * @paran resikved usages
   * @return bunch of possible definitions
   */
  std::vector<model::PythonAstNode> queryPossibleUsages(
    const model::PythonAstNode & astNode_,
    const std::vector<model::PythonBinding> & resolvedUsages_ = {}) const;

  /**
   * Get usage of the given ast node.
   * @param ast node id
   * @return bunch of nodes
   */
  std::vector<model::PythonAstNode> queryUsageByAstNode(
    const model::PythonAstNode::pktype & astNodeId_) const;

  /**
   * Query with limit and offset clause.
   */
  template<typename QueryType>
  odb::result<QueryType> queryWithLimit(
    const odb::query<QueryType>& query,
    const int limit,
    const int offset = 0) const
  {
    return db->query<QueryType>(
       query + " LIMIT " + odb::query<QueryType>::_val(limit)
            + " OFFSET " + odb::query<QueryType>::_val(offset));
  }

  /**
   * Create AstNodeInfo.
   * @return AstNodeInfo
   */
  const AstNodeInfo createAstNodeInfoByAstNodeId(
    const model::PythonAstNode::pktype & astNodeId_) const;

  /**
   * Create AstNodeInfo by binding id.
   * @return AstNodeInfo
   */
  const AstNodeInfo createAstNodeInfoByBindingId(
    const model::PythonBinding::pktype & astNodeId_) const;

  /**
   * Return a binning formatted string.
   */
  const std::string getFormattedQName(
    const model::PythonBinding& binding) const;

  /**
   * Returns mangled name from VariableRef table for an AstNode.
   *
   * If mangled name is not found, the return value will be an empty string.
   *
   * @param astNodeId_ an AST node id.
   * @return mangled name or empty string on error
   */
  const std::string getVarRefMangledName(
    const model::PythonAstNode::pktype& astNodeId_) const;

  /**
   * Queries the PythonVariableRef table by the following alogorthm:
   *   1, Finds the node's mangled name by the given astNodeId_
   *   2, Queries entries by mangled name + the given extra conditions
   *
   * @param astNodeId_ an AST node id.
   * @param extraCond_ optional extra conditions
   * @return a result set
   */
  odb::result<model::PythonVariableRef> getVarRefsForAstNode(
    const model::PythonAstNode::pktype& astNodeId_,
    const queryVariableRef extraCond_ = queryVariableRef()) const;

  /**
   * Return The Function call diagram for an ast node.
   * @return the svg formatted string
   */
  std::string getFunctionCallDiagram(
    const model::PythonAstNode& astNode_) const;

  /**
   * Create AstNodeInfo.
   * The type parameter Entity has to have 'FileLoc location' member.
   * @param entity
   * @return AstNodeInfo
   */
  template<typename EntityType>
  const AstNodeInfo createAstNodeInfo(
    const EntityType & entity) const;

  core::RangedHitCountResult makeRangedHitCountResult(
    const std::set<model::FileId>& resultSet,
    const int pageSize,
    const int pageNo) const;

  core::RangedHitCountResult getReferencesPage(
    const core::AstNodeId& astNodeId,
    const int pageSize,
    const int pageNo) const;

  /**
   * Nested query predicate which is evaulated true if the given position
   * is included by the given range.
   * @param range
   * @return nested query predicate
   */
  template<typename returnQueryType, typename RangeType>
  inline returnQueryType predContainsPosition(
    const RangeType & range_,
    const cc::service::core::Position & pos_) const
  {
    return {
      ( // start before the given position
        (range_.start.line < pos_.line) ||
        (range_.start.line == pos_.line &&
        range_.start.column <= pos_.column)
      ) &&
      ( // end after the given position
        (range_.end.line > pos_.line) ||
        (range_.end.line == pos_.line &&
        range_.end.column >= pos_.column)
      )
    };
  }

  /**
   * Nested query predicate which is evaulated true if the rhs range
   * is included by the lhs range.
   * @param lhs range
   * @param rhs range
   * @return nested query predicate
   */
  template<typename returnQueryType, typename FileLocType>
  inline returnQueryType predContainsLocationRange(
    const cc::model::FileLoc & lhs_,
    const FileLocType & rhs_) const
  {
    return {
      // in the same file
      rhs_.file == lhs_.file.object_id()
      &&
      ( // start before the given position
        (lhs_.range.start.line < rhs_.range.start.line) ||
        (lhs_.range.start.line == rhs_.range.start.line &&
        lhs_.range.start.column <= rhs_.range.start.column)
      ) &&
      ( // end after the given position
        (lhs_.range.end.line > rhs_.range.end.line) ||
        (lhs_.range.end.line == rhs_.range.end.line &&
        lhs_.range.end.column >= rhs_.range.end.column)
      )};
  }

//
// Member variables

private:
  std::shared_ptr<odb::database> db;


//
// Helper methods


  /**
   * Returns an iterator pointing to the element with the smallest value
   * in the range [first,last).
   * @param first element of the range
   * @param last element of the range
   * @return iterator pointing to the smallest element
   */
  template<typename ReturnType, typename InputIterator, typename Compare>
  ReturnType min_element(
    InputIterator first,
    InputIterator last,
    Compare comp) const
  {
    if (first==last)
      throw std::runtime_error("Empty sequence is given.");

    ReturnType smallest = *first;
    while (++first!=last)
      if(comp(*first,smallest))
        smallest = *first;
    return smallest;
  }

  /**
   * Create FileLoc from object which has the appropriate datas.
   */
  template<typename EntityType>
  model::FileLoc createFileLoc(const EntityType & entity) const
  {
    odb::lazy_shared_ptr<model::File> file(*db, entity.location_file);
    return {
      { // Range
        {entity.location_range_start_line, entity.location_range_start_column},
        {entity.location_range_end_line, entity.location_range_end_column}
      },
      file };
  }

  /**
   *
   */
  template<typename TableType, typename PredType>
  void insertRefs(
    const PredType& pred,
    const model::PythonAstNode::pktype & astNodeId_,
    std::set<model::PythonAstNode>& collection) const
  {
    auto result = db->query<TableType>(
      pred == astNodeId_);

    std::for_each(result.begin(), result.end(),
      [&collection, this](const TableType& element)
      {
        model::PythonAstNode node = { element.id, element.name, element.abv_qname, element.ast_type,
          createFileLoc(element), element.base_binding, element.container_binding,
          element.global_write };

        collection.insert(node);
      });
  }

};

} // python
} // language
} // service
} // cc

#endif // SERVICE_PYTHONSERVICE_PYTHONQUERYHELPER_H
