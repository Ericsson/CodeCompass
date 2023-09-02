#ifndef CODECOMPASS_PYTHONPERSISTENCE_H
#define CODECOMPASS_PYTHONPERSISTENCE_H

#include <parser/parsercontext.h>

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>

#include <util/hash.h>
#include <util/odbtransaction.h>

#include <parser/sourcemanager.h>

#include <odb/database.hxx>

#include <model/file.h>
#include <model/fileloc.h>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>

#include <model/pythonastnode.h>
#include <model/pythonastnode-odb.hxx>
#include <model/pythonclass.h>
#include <model/pythonclass-odb.hxx>
#include <model/pythondocumentation.h>
#include <model/pythondocumentation-odb.hxx>
#include <model/pythonfunction.h>
#include <model/pythonfunction-odb.hxx>
#include <model/pythoninheritance.h>
#include <model/pythoninheritance-odb.hxx>
#include <model/pythonimport.h>
#include <model/pythonimport-odb.hxx>
#include <model/pythontype.h>
#include <model/pythontype-odb.hxx>
#include <model/pythonvariable.h>
#include <model/pythonvariable-odb.hxx>
#include <model/pythonentity-odb.hxx>

namespace cc {
namespace parser {

class PythonPersistence {
private:
    struct AstNodeEquality {
    public:
        bool operator()(const model::PythonAstNodePtr& first, const model::PythonAstNodePtr& second) const {
            return first->id == second->id;
        }
    };

    struct AstNodeHash {
    public:
        size_t operator()(const model::PythonAstNodePtr& node) const {
            return node->id;
        }
    };

    using AstNodeSet = std::unordered_set<model::PythonAstNodePtr, AstNodeHash, AstNodeEquality>;

    ParserContext &ctx;

public:
    PythonPersistence(ParserContext &ctx_) : ctx(ctx_) {}

    ~PythonPersistence();

    void cppprint(boost::python::object o);

    void logInfo(boost::python::object o);
    void logWarning(boost::python::object o);
    void logError(boost::python::object o);
    void logDebug(boost::python::object o);

    void persistFile(boost::python::object pyFile);

    void persistVariable(boost::python::object pyVariable);

    void persistFunction(boost::python::object pyFunction);

    void persistPreprocessedClass(boost::python::object pyClass);

    void persistClass(boost::python::object pyClass);

    void persistImport(boost::python::object pyImport);

private:
    boost::optional <model::FileLoc> createFileLoc(boost::python::object filePosition);

    model::PythonEntityPtr getPythonEntity(const std::string &qualifiedName);

    model::PythonClassPtr getPythonClass(const std::string &qualifiedName);

    model::PythonVariablePtr getPythonVariable(const std::string &qualifiedName);

    bool isAstNodePersisted(const model::PythonAstNodePtr &node) const;

    bool isAstNodePersisted(const std::vector <model::PythonAstNodePtr> &nodes,
                            const model::PythonAstNodePtr &node) const;

    AstNodeSet _astNodes;
    std::vector<model::PythonVariablePtr> _variables;
    std::map<model::PythonEntityId, AstNodeSet> _variableUsages;
    std::vector<model::PythonFunctionPtr> _functions;
    std::map<model::PythonEntityId, AstNodeSet> _functionUsages;
    std::vector<model::PythonClassPtr> _classes;
    std::map<model::PythonEntityId, AstNodeSet> _classUsages;
    std::vector<model::PythonClassMemberPtr> _members;
    std::vector<model::PythonInheritancePtr> _inheritance;
    std::vector<model::PythonImportPtr> _imports;
    std::vector<model::PythonDocumentationPtr> _documentations;
    std::vector<model::PythonTypePtr> _types;
};

typedef boost::shared_ptr <PythonPersistence> PythonPersistencePtr;

}
}

#endif //CODECOMPASS_PYTHONPERSISTENCE_H
