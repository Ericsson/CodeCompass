#ifndef CC_SERVICE_PYTHON_PYTHONSERVICE_H
#define CC_SERVICE_PYTHON_PYTHONSERVICE_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <LanguageService.h>

#include <model/pyname.h>
#include <model/pyname-odb.hxx>

namespace cc
{
namespace service
{
namespace language
{

class PythonServiceHandler : virtual public LanguageServiceIf
{
public:
   PythonServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getFileTypes(std::vector<std::string>& return_) override;

  void getAstNodeInfo(
    AstNodeInfo& return_,
    const core::AstNodeId& astNodeId_) override;

  void getAstNodeInfoByPosition(
    AstNodeInfo& return_,
    const core::FilePosition& fpos_) override;

  void getSourceText(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override;

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

  std::int32_t getReferenceCount(
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_) override;

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

  std::int32_t getFileReferenceCount(
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override;

  void getSyntaxHighlight(
    std::vector<SyntaxHighlight>& return_,
    const core::FileRange& range_) override;

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
      the nodes of which the entity hash is identical to the queried one. */

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

    RETURN_TYPE, /*!< This option returns the return type of a function. */

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

    EXPANSION, /*!< Macro expansion. */

    UNDEFINITION, /*!< Macro undefinition. */
  };

  enum FileReferenceType
  {
    INCLUDES, /*!< Included source files in the current source file after the
      inclusion directive. */

    TYPES, /*!< User defined data types such as classes, structs etc. */

    FUNCTIONS, /*!< Functions in the current source file. */

    MACROS, /*!< Macros in the current source file. */
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

    CLASS_COLLABORATION, /*!< This returns a class collaboration diagram
      which shows the individual class members and their inheritance
      hierarchy. */

    COMPONENT_USERS, /*!< Component users diagram for source file S shows which
      source files depend on S through the interfaces S provides. */

    EXTERNAL_DEPENDENCY, /*!< This diagram shows the module which directory
      depends on. The "depends on" diagram on module A traverses the
      subdirectories of module A and shows all directories that contain files
      that any of the source files in A includes. */

    EXTERNAL_USERS, /*!< This diagram shows directories (modules) that are
      users of the queried module. */

    INCLUDE_DEPENDENCY, /*!< This diagram shows of the `#include` file
      dependencies. */

    INTERFACE, /*!< Interface diagram shows the used and provided interfaces of
      a source code file and shows linking information. */

    SUBSYSTEM_DEPENDENCY, /*!< This diagram shows the directories relationship
      between the subdirectories of the queried module. This diagram is useful
      to understand the relationships of the subdirectories (submodules)
      of a module. */
  };

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  std::shared_ptr<std::string> _datadir;
  const cc::webserver::ServerContext& _context;
};

} // language
} // service
} // cc

#endif // CC_SERVICE_PYTHON_PYTHONSERVICE_H
