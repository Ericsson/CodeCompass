#include <iostream>
#include <Python.h>

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>

#include <odb/database.hxx>

#include <pythonparser/pythonparser.h>

#include <util/hash.h>
#include <util/odbtransaction.h>

#include <parser/sourcemanager.h>

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
namespace parser{

void print(const std::string& s) {
    std::cout << s << std::endl;
}

class Persistence
{
private:
    ParserContext& ctx;

public:
    Persistence(ParserContext& ctx_) : ctx(ctx_) {}
    ~Persistence();

    void f() { std::cout << "C++" << std::endl; }
    void g(boost::python::object o);
    void persistFile(boost::python::object pyFile);
    void persistVariable(boost::python::object pyVariable);
    void persistFunction(boost::python::object pyFunction);
    void persistClass(boost::python::object pyClass);
    void persistImport(boost::python::object pyImport);

private:
    boost::optional<model::FileLoc> createFileLocFromPythonFilePosition(boost::python::object filePosition);
    boost::optional<model::PythonEntity> getPythonEntity(const std::string& qualifiedName);

    std::vector<model::PythonAstNodePtr> _astNodes;
    std::vector<model::PythonVariablePtr> _variables;
    std::vector<model::PythonFunctionPtr> _functions;
    std::vector<model::PythonClassPtr> _classes;
    std::vector<model::PythonClassMemberPtr> _members;
    std::vector<model::PythonInheritancePtr> _inheritance;
    std::vector<model::PythonImportPtr> _imports;
    std::vector<model::PythonDocumentationPtr> _documentations;
    std::vector<model::PythonTypePtr> _types;
};

Persistence::~Persistence()
{
    ctx.srcMgr.persistFiles();

    (util::OdbTransaction(ctx.db))([this]{
      util::persistAll(_astNodes, ctx.db);
      util::persistAll(_variables, ctx.db);
      util::persistAll(_functions, ctx.db);
      util::persistAll(_classes, ctx.db);
      util::persistAll(_members, ctx.db);
      util::persistAll(_inheritance, ctx.db);
      util::persistAll(_imports, ctx.db);
      util::persistAll(_documentations, ctx.db);
      util::persistAll(_types, ctx.db);
    });
}

void Persistence::g(boost::python::object o) {
    std::string s = boost::python::extract<std::string>(o);
    std::cout << s << std::endl;
}

void Persistence::persistFile(boost::python::object pyFile)
{
    try{
        model::FilePtr file = nullptr;
        model::BuildSource buildSource;

        boost::python::object path = pyFile.attr("path");
        boost::python::object status = pyFile.attr("parse_status");

        if(status.is_none()){
            std::cout << "status is None..." << std::endl;
        } else if(path.is_none()){
            std::cout << "path is None..." << std::endl;
        } else {
            buildSource.file = ctx.srcMgr.getFile(boost::python::extract<std::string>(path));
            std::cout << buildSource.file << std::endl;
            switch(boost::python::extract<int>(status)){
                case 0:
                    buildSource.file->parseStatus = model::File::PSNone;
                    break;
                case 1:
                    buildSource.file->parseStatus = model::File::PSPartiallyParsed;
                    break;
                case 2:
                    buildSource.file->parseStatus = model::File::PSFullyParsed;
                    break;
                default:
                    std::cout << "Unknown status: " << boost::python::extract<int>(status) << std::endl;
            }

            model::BuildActionPtr buildAction(new model::BuildAction);
            buildAction->command = "";
            buildAction->type = model::BuildAction::Other;
            try{
                util::OdbTransaction transaction{ctx.db};

                transaction([&, this] {
                    ctx.db->persist(buildAction);
                });

                buildSource.action = buildAction;

                ctx.srcMgr.updateFile(*buildSource.file);

                transaction([&, this] {
                    ctx.db->persist(buildSource);
                });
            } catch(const std::exception& ex){
                std::cout << "Exception: " << ex.what() << " - " << typeid(ex).name() << std::endl;
            }
        }
    } catch(std::exception e){
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistVariable(boost::python::object pyVariable)
{
    try{
        boost::python::object name = pyVariable.attr("name");
        boost::python::object qualifiedName = pyVariable.attr("qualified_name");
        boost::python::object visibility = pyVariable.attr("visibility");

        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyVariable.attr("file_position"));

        boost::python::list types = boost::python::extract<boost::python::list>(pyVariable.attr("type"));

        boost::python::list usages = boost::python::extract<boost::python::list>(pyVariable.attr("usages"));

        if(name.is_none() || qualifiedName.is_none() || fileLoc == boost::none ||
                types.is_none() || usages.is_none()){
            return;
        }

        util::OdbTransaction transaction{ctx.db};

        model::PythonAstNodePtr varAstNode(new model::PythonAstNode);
        varAstNode->location = fileLoc.get();
        varAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        varAstNode->symbolType = model::PythonAstNode::SymbolType::Variable;
        varAstNode->astType = model::PythonAstNode::AstType::Declaration;

        varAstNode->id = model::createIdentifier(*varAstNode);
        // transaction([&, this] {
        //     ctx.db->persist(*varAstNode);
        // });
        _astNodes.push_back(varAstNode);
        model::PythonVariablePtr variable(new model::PythonVariable);
        variable->astNodeId = varAstNode->id;
        variable->name = boost::python::extract<std::string>(name);
        variable->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        variable->visibility = boost::python::extract<std::string>(visibility);

        variable->id = model::createIdentifier(*variable);
        // transaction([&, this] {
        //     ctx.db->persist(*variable);
        // });
        _variables.push_back(variable);
        for(int i = 0; i<boost::python::len(types); ++i) {
            model::PythonTypePtr type(new model::PythonType);
            type->type = getPythonEntity(boost::python::extract<std::string>(types[i])).get().id;
            type->symbol = variable->id;
            // transaction([&, this] {
            //     ctx.db->persist(type);
            // });
            _types.push_back(type);
        }

        for(int i = 0; i<boost::python::len(usages); ++i){
            boost::optional<model::FileLoc> usageFileLoc =
                    createFileLocFromPythonFilePosition(usages[i].attr("file_position"));
            if(usageFileLoc == boost::none){
                continue;
            }
            model::PythonAstNodePtr usageAstNode(new model::PythonAstNode);
            usageAstNode->location = usageFileLoc.get();
            usageAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
            usageAstNode->symbolType = model::PythonAstNode::SymbolType::Variable;
            usageAstNode->astType = model::PythonAstNode::AstType::Usage;

            // transaction([&, this] {
            //     ctx.db->persist(usageAstNode);
            // });
            _astNodes.push_back(usageAstNode);
        }

    } catch (const odb::object_already_persistent& ex)
    {
        std::cout << "Var exception already persistent:" << std::endl;
        std::cout << ex.what() << std::endl;
    } catch (const odb::database_exception& ex)
    {
        std::cout << "Var exception db exception:" << std::endl;
        std::cout << ex.what() << std::endl;
    } catch(std::exception ex){
        std::cout << "Var exception:" << std::endl;
        std::cout << ex.what() << std::endl;
    }
}

void Persistence::persistFunction(boost::python::object pyFunction)
{
    try{
        boost::python::object name = pyFunction.attr("name");
        boost::python::object qualifiedName = pyFunction.attr("qualified_name");
        boost::python::object visibility = pyFunction.attr("visibility");

        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyFunction.attr("file_position"));

        boost::python::list types = boost::python::extract<boost::python::list>(pyFunction.attr("type"));

        boost::python::list usages = boost::python::extract<boost::python::list>(pyFunction.attr("usages"));

        boost::python::object pyDocumentation = pyFunction.attr("documentation");

        if(name.is_none() || qualifiedName.is_none() || fileLoc == boost::none ||
           types.is_none() || usages.is_none() || pyDocumentation.is_none()){
            return;
        }

        util::OdbTransaction transaction{ctx.db};

        model::PythonAstNodePtr funcAstNode(new model::PythonAstNode);
        funcAstNode->location = fileLoc.get();
        funcAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        funcAstNode->symbolType = model::PythonAstNode::SymbolType::Function;
        funcAstNode->astType = model::PythonAstNode::AstType::Declaration;

        funcAstNode->id = model::createIdentifier(*funcAstNode);

        // transaction([&, this] {
        //     ctx.db->persist(funcAstNode);
        // });
        _astNodes.push_back(funcAstNode);

        model::PythonFunctionPtr function(new model::PythonFunction);
        function->astNodeId = funcAstNode->id;
        function->name = boost::python::extract<std::string>(name);
        function->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        function->visibility = boost::python::extract<std::string>(visibility);

        function->id = model::createIdentifier(*function);

        // transaction([&, this] {
        //     ctx.db->persist(function);
        // });
        _functions.push_back(function);

        model::PythonDocumentationPtr documentation(new model::PythonDocumentation);
        documentation->documentation = boost::python::extract<std::string>(pyDocumentation);
        documentation->documented = function->id;
        documentation->documentationKind = model::PythonDocumentation::Function;

        // transaction([&, this] {
        //     ctx.db->persist(documentation);
        // });
        _documentations.push_back(documentation);

        for(int i = 0; i<boost::python::len(types); ++i) {
            model::PythonTypePtr type(new model::PythonType);
            type->type = getPythonEntity(boost::python::extract<std::string>(types[i])).get().id;
            type->symbol = function->id;

            // transaction([&, this] {
            //     ctx.db->persist(type);
            // });
            _types.push_back(type);
        }

        for(int i = 0; i<boost::python::len(usages); ++i){
            boost::optional<model::FileLoc> usageFileLoc =
                    createFileLocFromPythonFilePosition(usages[i].attr("file_position"));
            if(usageFileLoc == boost::none){
                continue;
            }
            model::PythonAstNodePtr usageAstNode(new model::PythonAstNode);
            usageAstNode->location = usageFileLoc.get();
            usageAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
            usageAstNode->symbolType = model::PythonAstNode::SymbolType::Function;
            usageAstNode->astType = model::PythonAstNode::AstType::Usage;

            // transaction([&, this] {
            //     ctx.db->persist(usageAstNode);
            // });
            _astNodes.push_back(usageAstNode);
        }
    } catch(std::exception e){
        std::cout << "Func exception:" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistClass(boost::python::object pyClass)
{
    try{
        boost::python::object name = pyClass.attr("name");
        boost::python::object qualifiedName = pyClass.attr("qualified_name");
        boost::python::object visibility = pyClass.attr("visibility");

        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyClass.attr("file_position"));

        boost::python::list usages = boost::python::extract<boost::python::list>(pyClass.attr("usages"));

        boost::python::object pyDocumentation = pyClass.attr("documentation");

        boost::python::list baseClasses = boost::python::extract<boost::python::list>(pyClass.attr("base_classes"));

        boost::python::object members = pyClass.attr("members");

        if(name.is_none() || qualifiedName.is_none() || fileLoc == boost::none ||
                usages.is_none() || pyDocumentation.is_none() || baseClasses.is_none() || members.is_none()){
            return;
        }

        util::OdbTransaction transaction{ctx.db};

        model::PythonAstNodePtr classAstNode(new model::PythonAstNode);
        classAstNode->location = fileLoc.get();
        classAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        classAstNode->symbolType = model::PythonAstNode::SymbolType::Class;
        classAstNode->astType = model::PythonAstNode::AstType::Declaration;

        classAstNode->id = model::createIdentifier(*classAstNode);

        // transaction([&, this] {
        //     ctx.db->persist(classAstNode);
        // });
        _astNodes.push_back(classAstNode);

        model::PythonClassPtr cl(new model::PythonClass);
        cl->astNodeId = classAstNode->id;
        cl->name = boost::python::extract<std::string>(name);
        cl->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        cl->visibility = boost::python::extract<std::string>(visibility);

        cl->id = model::createIdentifier(*cl);

        // transaction([&, this] {
        //     ctx.db->persist(cl);
        // });
        _classes.push_back(cl);

        model::PythonDocumentationPtr documentation(new model::PythonDocumentation);
        documentation->documentation = boost::python::extract<std::string>(pyDocumentation);
        documentation->documented = cl->id;
        documentation->documentationKind = model::PythonDocumentation::Class;

        // transaction([&, this] {
        //     ctx.db->persist(documentation);
        // });
        _documentations.push_back(documentation);

        for(int i = 0; i<boost::python::len(usages); ++i){
            boost::optional<model::FileLoc> usageFileLoc =
                    createFileLocFromPythonFilePosition(usages[i].attr("file_position"));
            if(usageFileLoc == boost::none){
                continue;
            }
            model::PythonAstNodePtr usageAstNode(new model::PythonAstNode);
            usageAstNode->location = usageFileLoc.get();
            usageAstNode->qualifiedName = boost::python::extract<std::string>(qualifiedName);
            usageAstNode->symbolType = model::PythonAstNode::SymbolType::Class;
            usageAstNode->astType = model::PythonAstNode::AstType::Usage;

            // transaction([&, this] {
            //     ctx.db->persist(usageAstNode);
            // });
            _astNodes.push_back(usageAstNode);
        }

        for(int i = 0; i<boost::python::len(baseClasses); ++i){
            model::PythonInheritancePtr inheritance(new model::PythonInheritance);
            inheritance->derived = cl->id;
            inheritance->base = getPythonEntity(boost::python::extract<std::string>(baseClasses[i])).get().id;

            // transaction([&, this] {
            //     ctx.db->persist(inheritance);
            // });
            _inheritance.push_back(inheritance);
        }

        boost::python::list methods = boost::python::extract<boost::python::list>(members.attr("methods"));
        boost::python::list staticMethods = boost::python::extract<boost::python::list>(members.attr("static_methods"));
        boost::python::list attributes = boost::python::extract<boost::python::list>(members.attr("attributes"));
        boost::python::list staticAttributes = boost::python::extract<boost::python::list>(members.attr("static_attributes"));
        boost::python::list classes = boost::python::extract<boost::python::list>(members.attr("classes"));

        if(methods.is_none() || staticMethods.is_none() || attributes.is_none() ||
                staticAttributes.is_none() || classes.is_none()){
            return;
        }

        for(int i = 0; i<boost::python::len(methods); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
            classMember->memberId = getPythonEntity(boost::python::extract<std::string>(methods[i])).get().id;
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Method;
            classMember->staticMember = false;

            // transaction([&, this] {
            //     ctx.db->persist(classMember);
            // });
            _members.push_back(classMember);
        }

        for(int i = 0; i<boost::python::len(staticMethods); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
            classMember->memberId = getPythonEntity(boost::python::extract<std::string>(staticMethods[i])).get().id;
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Method;
            classMember->staticMember = true;

            // transaction([&, this] {
            //     ctx.db->persist(classMember);
            // });
            _members.push_back(classMember);
        }

        for(int i = 0; i<boost::python::len(attributes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
            classMember->memberId = getPythonEntity(boost::python::extract<std::string>(attributes[i])).get().id;
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Attribute;
            classMember->staticMember = false;

            // transaction([&, this] {
            //     ctx.db->persist(classMember);
            // });
            _members.push_back(classMember);
        }

        for(int i = 0; i<boost::python::len(staticAttributes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
            classMember->memberId = getPythonEntity(boost::python::extract<std::string>(staticAttributes[i])).get().id;
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Attribute;
            classMember->staticMember = true;

            // transaction([&, this] {
            //     ctx.db->persist(classMember);
            // });
            _members.push_back(classMember);
        }

        for(int i = 0; i<boost::python::len(classes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
            classMember->memberId = getPythonEntity(boost::python::extract<std::string>(classes[i])).get().id;
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Class;
            classMember->staticMember = false;

            // transaction([&, this] {
            //     ctx.db->persist(classMember);
            // });
            _members.push_back(classMember);
        }
    } catch(std::exception e){
        std::cout << "Class exception:" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistImport(boost::python::object pyImport)
{
    try {
        model::FilePtr file = ctx.srcMgr.getFile(boost::python::extract<std::string>(pyImport.attr("importer")));

        boost::python::list importedModules =
                boost::python::extract<boost::python::list>(pyImport.attr("imported_modules"));

        boost::python::dict importedSymbols =
                boost::python::extract<boost::python::dict>(pyImport.attr("imported_symbols"));

        if (file == nullptr || importedModules.is_none() || importedSymbols.is_none()) {
            return;
        }

        util::OdbTransaction transaction{ctx.db};

        for (int i = 0; i < boost::python::len(importedModules); ++i) {
            boost::python::object importData = importedModules[i];

            model::FilePtr moduleFile = ctx.srcMgr.getFile(boost::python::extract<std::string>(importData.attr("imported")));
            boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(importData.attr("position"));

            if (moduleFile == nullptr || fileLoc == boost::none) {
                continue;
            }

            model::PythonAstNodePtr moduleAstNode(new model::PythonAstNode);
            moduleAstNode->location = fileLoc.get();
            moduleAstNode->qualifiedName = "";
            moduleAstNode->symbolType = model::PythonAstNode::SymbolType::Module;
            moduleAstNode->astType = model::PythonAstNode::AstType::Declaration;

            moduleAstNode->id = model::createIdentifier(*moduleAstNode);

            model::PythonImportPtr moduleImport(new model::PythonImport);
            moduleImport->astNodeId = moduleAstNode->id;
            moduleImport->importer = file;
            moduleImport->imported = moduleFile;

            // transaction([&, this] {
            //     ctx.db->persist(moduleAstNode);
            //     ctx.db->persist(moduleImport);
            // });
            _astNodes.push_back(moduleAstNode);
            _imports.push_back(moduleImport);
        }

        boost::python::list importDict = importedSymbols.items();
        for (int i = 0; i < boost::python::len(importDict); ++i) {
            boost::python::tuple import = boost::python::extract<boost::python::tuple>(importDict[i]);

            boost::python::object importData = import[0];

            model::FilePtr moduleFile = ctx.srcMgr.getFile(boost::python::extract<std::string>(importData.attr("imported")));
            boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(importData.attr("position"));

            if (moduleFile == nullptr || fileLoc == boost::none) {
                continue;
            }

            model::PythonAstNodePtr moduleAstNode(new model::PythonAstNode);
            moduleAstNode->location = fileLoc.get();
            moduleAstNode->qualifiedName = "";
            moduleAstNode->symbolType = model::PythonAstNode::SymbolType::Module;
            moduleAstNode->astType = model::PythonAstNode::AstType::Declaration;

            moduleAstNode->id = model::createIdentifier(*moduleAstNode);

            for (int j = 0; j < boost::python::len(import[1]); ++j){
                model::PythonImportPtr moduleImport(new model::PythonImport);
                moduleImport->astNodeId = moduleAstNode->id;
                moduleImport->importer = file;
                moduleImport->imported = moduleFile;
                moduleImport->importedSymbol = getPythonEntity(boost::python::extract<std::string>(import[1][j])).get().id;

                // transaction([&, this] {
                //     ctx.db->persist(moduleImport);
                // });
                _imports.push_back(moduleImport);
            }

            // transaction([&, this] {
            //     ctx.db->persist(moduleAstNode);
            // });
            _astNodes.push_back(moduleAstNode);
        }
    } catch(std::exception e){
        std::cout << "Import exception:" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

boost::optional<model::FileLoc> Persistence::createFileLocFromPythonFilePosition(boost::python::object filePosition)
{
    if (filePosition.is_none()){
        return boost::none;
    }

    boost::python::object filePath = filePosition.attr("file");
    boost::python::object pyRange = filePosition.attr("range");

    if (filePath.is_none() || pyRange.is_none()){
        return boost::none;
    }

    boost::python::object pyStartPosition = pyRange.attr("start_position");
    boost::python::object pyEndPosition = pyRange.attr("end_position");

    if (pyStartPosition.is_none() || pyEndPosition.is_none()){
        return boost::none;
    }

    model::FileLoc fileLoc;

    fileLoc.file = ctx.srcMgr.getFile(boost::python::extract<std::string>(filePath));

    model::Position startPosition(boost::python::extract<int>(pyStartPosition.attr("line")),
                                boost::python::extract<int>(pyStartPosition.attr("column")));
    model::Position endPosition(boost::python::extract<int>(pyEndPosition.attr("line")),
                                boost::python::extract<int>(pyEndPosition.attr("column")));

    fileLoc.range = model::Range(startPosition, endPosition);

    return fileLoc;
}

boost::optional<model::PythonEntity> Persistence::getPythonEntity(const std::string& qualifiedName)
{
    auto varIt = std::find_if(_variables.begin(), _variables.end(), [&](const auto& var){ return var->qualifiedName == qualifiedName; });
    if (varIt != _variables.end()){
        return **varIt;
    }

    auto funcIt = std::find_if(_functions.begin(), _functions.end(), [&](const auto& func){ return func->qualifiedName == qualifiedName; });
    if (funcIt != _functions.end()){
        return **funcIt;
    }

    auto classIt = std::find_if(_classes.begin(), _classes.end(), [&](const auto& cl){ return cl->qualifiedName == qualifiedName; });
    if (classIt != _classes.end()){
        return **classIt;
    }

    using EntityQuery = odb::query<model::PythonEntity>;
    using EntityResult = odb::result<model::PythonEntity>;

    EntityResult entity = ctx.db->query<model::PythonEntity>(
            EntityQuery::qualifiedName == qualifiedName);

    if (entity.empty()){
        return boost::none;
    }

    return *entity.begin();
}

typedef boost::shared_ptr<Persistence> PersistencePtr;

BOOST_PYTHON_MODULE(persistence){
    boost::python::class_<Persistence>("Persistence", boost::python::init<ParserContext&>())
            .def("f", &Persistence::f)
            .def("g", &Persistence::g)
            .def("persist_file", &Persistence::persistFile)
            .def("persist_variable", &Persistence::persistVariable)
            .def("persist_function", &Persistence::persistFunction)
            .def("persist_class", &Persistence::persistClass)
            .def("persist_import", &Persistence::persistImport);
}

PythonParser::PythonParser(ParserContext &ctx_) : AbstractParser(ctx_) {}

PythonParser::~PythonParser()
{
}

void PythonParser::markModifiedFiles() {}

bool PythonParser::cleanupDatabase() { return true; }

bool PythonParser::parse()
{
    setenv("PYTHONPATH",
           "/home/rmfcnb/ELTE/Code-Compass-Python-Plugin/:"
           "/home/rmfcnb/CodeCompass/build/plugins/python/parser/:"
           "/usr/lib/python3.8/", 1);

    try{
        Py_Initialize();
        init_module_persistence();

        boost::python::object module = boost::python::import("my_ast.python_parser");

        if(!module.is_none()){
            boost::python::object func = module.attr("parse");

            if(!func.is_none() && PyCallable_Check(func.ptr())){
                std::string source_path;
                for (const std::string& input : _ctx.options["input"].as<std::vector<std::string>>()){
                    if (boost::filesystem::is_directory(input)){
                        source_path = input;
                    }
                }
                if(source_path.empty()){
                    std::cout << "No source path was found" << std::endl;
                } else {
                    PersistencePtr persistencePtr(new Persistence(_ctx));

                    func(source_path, boost::python::ptr(persistencePtr.get()));
                }
            } else {
                std::cout << "Cannot find function" << std::endl;
            }
        } else {
            std::cout << "Cannot import module" << std::endl;
        }
    }catch(boost::python::error_already_set){
        PyErr_Print();
    }

    // Py_Finalize();
    return true;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
boost::program_options::options_description getOptions()
{
    boost::program_options::options_description description("Python Plugin");
    description.add_options()
            ("skip-doccomment",
             "If this flag is given the parser will skip parsing the documentation "
             "comments.");
    return description;
}

std::shared_ptr<PythonParser> make(ParserContext& ctx_)
{
    return std::make_shared<PythonParser>(ctx_);
}
}
#pragma clang diagnostic pop

}
}