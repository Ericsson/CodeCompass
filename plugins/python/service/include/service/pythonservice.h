#ifndef CC_SERVICE_LANGUAGE_PYTHONSERVICE_H
#define CC_SERVICE_LANGUAGE_PYTHONSERVICE_H

#include <LanguageService.h>

#include <map>

#include <odb/database.hxx>

#include <model/pythonastnode.h>
#include <model/pythonastnode-odb.hxx>
#include <model/pythonclass.h>
#include <model/pythonentity.h>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

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

        PARAMETER, /*!< This option returns the parameters of a function. */

        LOCAL_VAR, /*!< This option returns the local variables of a function. */

        RETURN_TYPE, /*!< This option returns the return type of a function. */

        TYPE, /*!< This option returns the type of a variable. */

        INHERIT_FROM, /*!< Types from which the queried type inherits. */

        INHERIT_BY, /*!< Types by which the queried type is inherited. */

        DATA_MEMBER, /*!< Data members of a class. */

        METHOD, /*!< Members of a class. */

        NESTED_CLASS, /*!< Nested classes. */

        IMPORTED_SYMBOLS
    };

    enum FileReferenceType
    {
        IMPORTS, /*!< Imported modules in the current module. */

        CLASSES, /*!< User defined classes */

        FUNCTIONS, /*!< Functions in the current module. */

        VARIABLES /*!< Variables in the current module. */
    };

    static bool compareByPosition(
            const model::PythonAstNode& lhs,
            const model::PythonAstNode& rhs);

    static bool compareByValue(
            const model::PythonAstNode& lhs,
            const model::PythonAstNode& rhs);

    model::PythonAstNode queryPythonAstNode(const core::AstNodeId& astNodeId_);

    std::vector<model::PythonAstNode> queryPythonAstNodes(
            const core::AstNodeId& astNodeId_,
            const odb::query<model::PythonAstNode>& query_
            = odb::query<model::PythonAstNode>(true));

    std::vector<model::PythonAstNode> queryPythonAstNodesInFile(
            const core::FileId& fileId_,
            const odb::query<model::PythonAstNode>& query_
            = odb::query<model::PythonAstNode>(true));

    std::uint32_t queryPythonAstNodeCountInFile(
            const core::FileId& fileId_,
            const odb::query<model::PythonAstNode>& query_
            = odb::query<model::PythonAstNode>(true));

    std::vector<model::PythonAstNode> queryDeclarations(const core::AstNodeId& astNodeId_);

    odb::query<model::PythonAstNode> astCallsQuery(const model::PythonAstNode& astNode_);

    std::vector<model::PythonAstNode> queryCalls(const core::AstNodeId& astNodeId_);

    std::size_t queryPythonAstNodeCount(
            const core::AstNodeId& astNodeId_,
            const odb::query<model::PythonAstNode>& query_
            = odb::query<model::PythonAstNode>(true));

    std::size_t queryCallsCount(const core::AstNodeId& astNodeId_);

    std::vector<model::PythonClass> queryTypes(const model::PythonEntity& entity);

    model::PythonEntity queryPythonEntity(const model::PythonEntityId& id);

    model::PythonEntity queryPythonEntityByAstNode(const std::string& qualifiedName);

    std::map<model::PythonAstNodeId, std::string> getVisibilities(const std::vector<model::PythonAstNode>& nodes_);

    bool isGeneratedVariable(const model::PythonAstNode& node_) const;

    std::shared_ptr<odb::database> _db;
    util::OdbTransaction _transaction;

    std::shared_ptr<std::string> _datadir;
    const cc::webserver::ServerContext& _context;
};

}
}
}

#endif
