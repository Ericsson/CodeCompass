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
public:
  CppServiceHandler(
    std::shared_ptr<odb::database> db_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());

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
    std::vector<core::Description>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagram(
    std::string& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t diagramId_) override;

  void getDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getFileDiagramTypes(
    std::vector<core::Description>& return_,
    const core::FileId& fileId_) override;

  void getFileDiagram(
    std::string& return_,
    const core::FileId& fileId_,
    const int32_t diagramId_) override;

  void getFileDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getReferenceTypes(
    std::vector<core::Description>& return_,
    const core::AstNodeId& astNodeId) override;

  void getReferences(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_) override;

  void getReferencesInFile(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const core::FileId& fileId_) override;

  void getReferencesPage(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::int32_t pageSize_,
    const std::int32_t pageNo_) override;

  void getFileReferenceTypes(
    std::vector<core::Description>& return_,
    const core::FileId& fileId_) override;

  void getFileReferences(
    std::vector<core::FileInfo>& return_,
    const core::FileId& fileId,
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

    CALLEE, /*!< Get called functions. WARNING: If the definition of the AST
      node is not unique then it returns the callees of one of them. */

    CALLER, /*!< Get caller functions. */
    
    VIRTUAL_CALL, /*!< A function may be used virtually on a base type object.
      The exact type of the object is based on dynamic information, which can't
      be determined statically. Weak usage returns these possible calls. */

    FUNC_PTR_CALL, /*!< Functions can be assigned to function pointers which
      can be invoked later. This option returns these invocations. */

    PARAMETER, /*!< This option returns the parameters of a function. */

    LOCAL_VAR, /*!< This option returns the local variables of a function. */

    READ, /*!< This option returns the places where a variable is read. */

    WRITE, /*!< This option returns the places where a variable is written. */

    ALIAS, /*!< Types may have aliases, e.g. by typedefs. */

    PUBLIC_INHERIT_FROM, /*!< Types from which the queried type inherits and the
      inheritance is public. */

    PRIVATE_INHERIT_FROM, /*!< Types from which the queried type inherits and
      the inheritance is private. */

    PROTECTED_INHERIT_FROM, /*!< Types from which the queried type inherits and
      the inheritance is protected. */

    PUBLIC_INHERIT_BY, /*!< Types by which the queried type is inherited and the
      inheritance is public. */

    PRIVATE_INHERIT_BY, /*!< Types by which the queried type is inherited and
      the inheritance is private. */

    PROTECTED_INHERIT_BY, /*!< Types by which the queried type is inherited and
      the inheritance is protected. */

    PUBLIC_MEMBER, /*!< Public members of a class. */

    PRIVATE_MEMBER, /*!< Private members of a class. */

    PROTECTED_MEMBER, /*!< Protected members of a class. */

    PUBLIC_METHOD, /*!< Public members of a class. */

    PRIVATE_METHOD, /*!< Private members of a class. */

    PROTECTED_METHOD, /*!< Protected members of a class. */

    FRIEND, /*!< The friends of a class. */

    USAGE_AS_GLOBAL, /*!< Usages of a type as the type of a global variable. */

    USAGE_AS_LOCAL, /*!< Usages of a type as the type of a local variable. */

    USAGE_AS_FIELD, /*!< Usages of a type as the type of a class field. */

    USAGE_AS_PARAMETER, /*!< Usages of a type as the type of a function
      parameter. */

    USAGE_AS_RETURN, /*!< Usages of a type as the type of a returned
      variable. */

    UNDERLYING_TYPE, /*!< Underlying type of a typedef. */

    ENUM_CONSTANTS, /*!< Enum constants. */
  };

  enum DiagramType
  {
    FUNCTION_CALL,
    DETAILED_CLASS,
    CLASS_OVERVIEW,
    CLASS_COLLABORATION
  };

  /**
   * This function transforms a model::CppAstNode to an AstNodeInfo Thrift
   * object.
   */
  static AstNodeInfo createAstNodeInfo(const model::CppAstNode& astNode_);

  static bool compareByPosition(
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
      = odb::query<model::CppAstNode>());

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
   * This function computes the reverse transitive closure of an element based
   * on the CppRelation table along the relations of a given kind.
   */
  std::unordered_set<std::uint64_t> reverseTransitiveClosureOfRel(
    model::CppRelation::Kind kind_,
    std::uint64_t to_);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
};

}
}
}

#endif
