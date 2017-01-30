#ifndef CC_SERVICE_LANGUAGE_CPPSERVICE_H
#define CC_SERVICE_LANGUAGE_CPPSERVICE_H

#include <memory>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <LanguageService.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cpprelation.h>
#include <model/cpprelation-odb.hxx>

#include <util/odbtransaction.h>

namespace cc
{
namespace service
{
namespace language
{

class CppServiceHandler : virtual public LanguageServiceIf
{
  friend class Diagram;

public:
  CppServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());

  void getFileTypes(std::vector<std::string>& return_) override;

  void getAstNodeInfo(
    AstNodeInfo& return_,
    const core::AstNodeId& astNodeId_) override;

  void getAstNodeInfoByPosition(
    AstNodeInfo& return_,
    const core::FilePosition& fpos_) override;

  void getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override;

  void getProperties(
    std::map<std::string, std::string>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagram(
    std::string& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t diagramId_) override;

  void getDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getFileDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override;

  void getFileDiagram(
    std::string& return_,
    const core::FileId& fileId_,
    const int32_t diagramId_) override;

  void getFileDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId) override;

  void getReferences(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::vector<std::string>& tags_) override;

  void getReferencesInFile(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const core::FileId& fileId_,
    const std::vector<std::string>& tags_) override;

  void getReferencesPage(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::int32_t pageSize_,
    const std::int32_t pageNo_) override;

  void getFileReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override;

  void getFileReferences(
    std::vector<AstNodeInfo>& return_,
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override;

  void getSyntaxHighlight(
    std::vector<SyntaxHighlight>& return_,
    const core::FileId& fileId) override;

private:
  enum ReferenceType
  {
    DEFINITION, /*!< By this option the definition(s) of the AST node can be
      queried. However according to the "one definition rule" a named entity
      can have only one definition, in a parsing several definitions might be
      available. This is the case when the project is built for several targets
      and in the different builds different definitions are defined for an
      entity (e.g. because of an #ifdef section). */

    DECLARATION, /*!< By this options the declaration(s) of the AST node can be
      queried. */

    USAGE, /*!< By this option the usages of the AST node can be queried, i.e.
      the nodes of which the mangled name is identical to the queried one. The
      parser creates a mangled name even for variables and types, which is
      unique for the element. */

    THIS_CALLS, /*!< Get function calls in a function. WARNING: If the
      definition of the AST node is not unique then it returns the callees of
      one of them. */

    CALLS_OF_THIS, /*!< Get calls of a function. */

    CALLEE, /*!< Get called functions definitions. WARNING: If the definition of
      the AST node is not unique then it returns the callees of one of them. */

    CALLER, /*!< Get caller functions. */

    VIRTUAL_CALL, /*!< A function may be used virtually on a base type object.
      The exact type of the object is based on dynamic information, which can't
      be determined statically. Weak usage returns these possible calls. */

    FUNC_PTR_CALL, /*!< Functions can be assigned to function pointers which
      can be invoked later. This option returns these invocations. */

    PARAMETER, /*!< This option returns the parameters of a function. */

    LOCAL_VAR, /*!< This option returns the local variables of a function. */

    OVERRIDE, /*!< This option returns the functions which the given function
      overrides. */

    OVERRIDDEN_BY, /*!< This option returns the overrides of a function. */

    READ, /*!< This option returns the places where a variable is read. */

    WRITE, /*!< This option returns the places where a variable is written. */

    TYPE, /*!< This option returns the type of a variable. */

    ALIAS, /*!< Types may have aliases, e.g. by typedefs. */

    INHERIT_FROM, /*!< Types from which the queried type inherits. */

    INHERIT_BY, /*!< Types by which the queried type is inherited. */

    DATA_MEMBER, /*!< Data members of a class. */

    METHOD, /*!< Members of a class. */

    FRIEND, /*!< The friends of a class. */

    UNDERLYING_TYPE, /*!< Underlying type of a typedef. */

    ENUM_CONSTANTS, /*!< Enum constants. */
  };

  enum FileReferenceType
  {
    INCLUDES, /*!< Included source files in the current source file after the
      inclusion directive. */

    TYPES, /*!< User defined data types such as classes, structs etc. */

    FUNCTIONS /*!< Functions in the current source file. */
  };

  enum DiagramType
  {
    FUNCTION_CALL, /*!< In the function call diagram the nodes are functions and
      the edges are the function calls between them. The diagram also displays
      some dynamic information such as virtual function calls. */

    DETAILED_CLASS, /*!< This is a classical UML class diagram for the selected
      class and its direct children and parents. The nodes contain the methods
      and member variables with their visibility. */

    CLASS_OVERVIEW, /*!< This is a class diagram which contains all classes
      which inherit from the current one, and all parents from which the
      current one inherits. The methods and member variables are node included
      in the nodes, but the type of the member variables are indicated as
      aggregation relationship. */

    CLASS_COLLABORATION /*!< This returns a class collaboration diagram
      which shows the individual class members and their inheritance hierarchy. */
  };

  static bool compareByPosition(
    const model::CppAstNode& lhs,
    const model::CppAstNode& rhs);

  /**
   * This function compares AST nodes alphabetically.
   */
  static bool compareByValue(
    const model::CppAstNode& lhs,
    const model::CppAstNode& rhs);

  /**
   * This function returns the corresponding model::CppAstNode to the given AST
   * node.
   */
  model::CppAstNode queryCppAstNode(const core::AstNodeId& astNodeId_);

  /**
   * This function returns the model::CppAstNode objects which meet the
   * requirements of the given query and have the same mangled name as the given
   * AST node.
   */
  std::vector<model::CppAstNode> queryCppAstNodes(
    const core::AstNodeId& astNodeId_,
    const odb::query<model::CppAstNode>& query_
      = odb::query<model::CppAstNode>(true));

    /**
     * This function returns the model::CppAstNode objects which meet the
     * requirements of the given query in the given file.
     */
    std::vector<model::CppAstNode> queryCppAstNodesInFile(
      const core::FileId& fileId_,
      const odb::query<model::CppAstNode>& query_
        = odb::query<model::CppAstNode>(true));

  /**
   * This function returns the model::CppAstNode objects which have the same
   * mangled name as the given astNodeId_ and have
   * model::CppAstNode::AstType::Definition type. Unfortunately the definition
   * of an entity is not unequivocal, since in different translation units
   * different definitions may occure for example because of an #ifdef block.
   */
  std::vector<model::CppAstNode> queryDefinitions(
    const core::AstNodeId& astNodeId_);

  /**
   * This function returns the function calls in a given function.
   * @param astNodeId_ An AST node ID which belongs to a function.
   */
  std::vector<model::CppAstNode> queryCalls(const core::AstNodeId& astNodeId_);

  /**
   * This function returns the functions which override the given one.
   * @param reverse_ If this parameter is true then the function returns the
   * functions which are overriden by the given one.
   */
  std::vector<model::CppAstNode> queryOverrides(
    const core::AstNodeId& astNodeId_,
    bool reverse_ = false);

  /**
   * This function computes the transitive closure of an element based on the
   * CppRelation table along the relations of a given kind.
   * @param reverse_ If true then the transitive closure is computed along the
   * reverse relation of the given one.
   */
  std::unordered_set<std::uint64_t> transitiveClosureOfRel(
    model::CppRelation::Kind kind_,
    std::uint64_t to_,
    bool reverse_ = false);

  /**
   * This function returns meta information of the AST nodes
   * (e.g. public, static, virtual etc.)
   */
  std::map<model::CppAstNodeId, std::vector<std::string>> getTags(
    const std::vector<model::CppAstNode>& nodes_);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  std::shared_ptr<std::string> _datadir;
  const boost::program_options::variables_map& _config;
};

}
}
}

#endif
