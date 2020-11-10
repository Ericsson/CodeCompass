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

namespace cc {
namespace parser{

class Persistence
{
private:
    ParserContext& ctx;

public:
    Persistence(ParserContext& ctx_) : ctx(ctx_) {}

    void f() { std::cout << "C++" << std::endl; }
    void persistFile(boost::python::object pyFile);
    void persistVariable(boost::python::object pyVariable);
    void persistFunction(boost::python::object pyFunction);
    void persistClass(boost::python::object pyClass);
    void persistImport(boost::python::object pyImport);

private:
    boost::optional<model::FileLoc> createFileLocFromPythonFilePosition(boost::python::object filePosition);
    model::PythonEntity* getPythonEntity(const std::string& qualifiedName, const model::FileLoc& fileLoc);
};

void Persistence::persistFile(boost::python::object pyFile)
{
    try{
        std::cout << "0" << std::endl;
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
        std::cout << "1" << std::endl;
        boost::python::object name = pyVariable.attr("name");
        boost::python::object qualifiedName = pyVariable.attr("qualified_name");
        boost::python::object visibility = pyVariable.attr("visibility");

        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyVariable.attr("file_position"));

        // set
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

        transaction([&, this] {
            ctx.db->persist(varAstNode);
        });

        model::PythonVariablePtr variable(new model::PythonVariable);
        variable->astNodeId = varAstNode->id;
        variable->name = boost::python::extract<std::string>(name);
        variable->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        variable->visibility = boost::python::extract<std::string>(visibility);

        variable->id = model::createIdentifier(*variable);

        transaction([&, this] {
            ctx.db->persist(variable);
        });

        for(int i = 0; i<boost::python::len(types); ++i) {
            model::PythonTypePtr type(new model::PythonType);
    //        type->type = getPythonEntityIdByQualifiedName(boost::python::extract<std::string>(types[i]));
            type->symbol = variable->id;

            transaction([&, this] {
                ctx.db->persist(type);
            });
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

            transaction([&, this] {
                ctx.db->persist(usageAstNode);
            });
        }
    } catch(std::exception e){
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistFunction(boost::python::object pyFunction)
{
    try{
        std::cout << "2" << std::endl;
        boost::python::object name = pyFunction.attr("name");
        boost::python::object qualifiedName = pyFunction.attr("qualified_name");
        boost::python::object visibility = pyFunction.attr("visibility");


        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyFunction.attr("file_position"));

        // set
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

        transaction([&, this] {
            ctx.db->persist(funcAstNode);
        });

        model::PythonFunctionPtr function(new model::PythonFunction);
        function->astNodeId = funcAstNode->id;
        function->name = boost::python::extract<std::string>(name);
        function->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        function->visibility = boost::python::extract<std::string>(visibility);

        function->id = model::createIdentifier(*function);

        transaction([&, this] {
            ctx.db->persist(function);
        });

        model::PythonDocumentationPtr documentation(new model::PythonDocumentation);
        documentation->documentation = boost::python::extract<std::string>(pyDocumentation);
        documentation->documented = function->id;
        documentation->documentationKind = model::PythonDocumentation::Function;

        transaction([&, this] {
            ctx.db->persist(documentation);
        });

        for(int i = 0; i<boost::python::len(types); ++i) {
            model::PythonTypePtr type(new model::PythonType);
    //        type->type = getPythonEntityIdByQualifiedName(boost::python::extract<std::string>(types[i]));
            type->symbol = function->id;

            transaction([&, this] {
                ctx.db->persist(type);
            });
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

            transaction([&, this] {
                ctx.db->persist(usageAstNode);
            });
        }
    } catch(std::exception e){
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistClass(boost::python::object pyClass)
{
    try{
        std::cout << "3" << std::endl;
        boost::python::object name = pyClass.attr("name");
        boost::python::object qualifiedName = pyClass.attr("qualified_name");
        boost::python::object visibility = pyClass.attr("visibility");

        boost::optional<model::FileLoc> fileLoc = createFileLocFromPythonFilePosition(pyClass.attr("file_position"));

        boost::python::list usages = boost::python::extract<boost::python::list>(pyClass.attr("usages"));

        boost::python::object pyDocumentation = pyClass.attr("documentation");

        // set
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

        transaction([&, this] {
            ctx.db->persist(classAstNode);
        });

        model::PythonClassPtr cl(new model::PythonClass);
        cl->astNodeId = classAstNode->id;
        cl->name = boost::python::extract<std::string>(name);
        cl->qualifiedName = boost::python::extract<std::string>(qualifiedName);
        cl->visibility = boost::python::extract<std::string>(visibility);

        cl->id = model::creatIdentifier(*cl);

        transaction([&, this] {
            ctx.db->persist(cl);
        });

        model::PythonDocumentationPtr documentation(new model::PythonDocumentation);
        documentation->documentation = boost::python::extract<std::string>(pyDocumentation);
        documentation->documented = cl->id;
        documentation->documentationKind = model::PythonDocumentation::Class;

        transaction([&, this] {
            ctx.db->persist(documentation);
        });

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

            transaction([&, this] {
                ctx.db->persist(usageAstNode);
            });
        }

        for(int i = 0; i<boost::python::len(baseClasses); ++i){
            model::PythonInheritancePtr inheritance(new model::PythonInheritance);
            inheritance->derived = cl->id;
    //        inheritance->base = getPythonEntityIdByQualifiedName(boost::python::extract<std::string>(baseClasses[i]));

            transaction([&, this] {
                ctx.db->persist(inheritance);
            });
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
    //        classMember->memberId = search for function declaration
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Method;
            classMember->staticMember = false;

            transaction([&, this] {
                ctx.db->persist(classMember);
            });
        }

        for(int i = 0; i<boost::python::len(staticMethods); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
    //        classMember->memberId = search for function declaration
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Method;
            classMember->staticMember = true;

            transaction([&, this] {
                ctx.db->persist(classMember);
            });
        }

        for(int i = 0; i<boost::python::len(attributes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
    //        classMember->memberId = search for variable declaration
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Attribute;
            classMember->staticMember = false;

            transaction([&, this] {
                ctx.db->persist(classMember);
            });
        }

        for(int i = 0; i<boost::python::len(staticAttributes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
    //        classMember->memberId = search for variable declaration
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Attribute;
            classMember->staticMember = true;

            transaction([&, this] {
                ctx.db->persist(classMember);
            });
        }

        for(int i = 0; i<boost::python::len(classes); ++i){
            model::PythonClassMemberPtr classMember(new model::PythonClassMember);
            classMember->astNodeId = classAstNode->id;
    //        classMember->memberId = search for class declaration
            classMember->classId = cl->id;
            classMember->kind = model::PythonClassMember::Class;
            classMember->staticMember = false;

            transaction([&, this] {
                ctx.db->persist(classMember);
            });
        }
    } catch(std::exception e){
        std::cout << e.what() << std::endl;
    }
}

void Persistence::persistImport(boost::python::object pyImport)
{
    try {
        std::cout << "4" << std::endl;
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
            model::FilePtr moduleFile = ctx.srcMgr.getFile(boost::python::extract<std::string>(importedModules[i]));
            if (moduleFile == nullptr) {
                continue;
            }
            model::PythonImportPtr moduleImport(new model::PythonImport);
            moduleImport->importer = file;
            moduleImport->imported = moduleFile;

            transaction([&, this] {
                ctx.db->persist(moduleImport);
            });
        }

        boost::python::list importDict = importedSymbols.items();
        for (int i = 0; i < boost::python::len(importDict); ++i) {
            boost::python::tuple import = boost::python::extract<boost::python::tuple>(importDict[i]);
            model::FilePtr moduleFile = ctx.srcMgr.getFile(boost::python::extract<std::string>(import[0]));
            if (moduleFile == nullptr) {
                continue;
            }
            model::PythonImportPtr moduleImport(new model::PythonImport);
            moduleImport->importer = file;
            moduleImport->imported = moduleFile;
//            moduleImport->importedSymbol = getPythonEntity(boost::python::extract<std::string>(import[1]));

            transaction([&, this] {
                ctx.db->persist(moduleImport);
            });
        }
    } catch(std::exception e){
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

model::PythonEntity* Persistence::getPythonEntity(const std::string& qualifiedName, const model::FileLoc& fileLoc)
{
    using odb::query<model::PythonAstNode> AstQuery;
    using odb::result<model::PythonAstNode> AstResult;
    using odb::query<model::PythonEntity> EntityQuery;
    using odb::result<model::PythonEntity> EntityResult;

    const model::Position& start = astNode_.location.range.start;
    const model::Position& end = astNode_.location.range.end;

    AstResult astNode = ctx.db->query<model::PythonAstNode>(
            AstQuery::location.file == astNode_.location.file.object_id() &&
            // StartPos == Pos
            (AstQuery::location.range.start.line == start.line &&
            AstQuery::location.range.start.column == start.column) &&
            // Pos == EndPos
            (AstQuery::location.range.end.line == end.line &&
            AstQuery::location.range.end.column == end.column));

    if (astNode.empty()){
        return nullptr;
    }

    EntityResult entity = ctx.db->query<model::PythonEntity>(
            EntityQuery::qualifiedName == qualifiedName &&
            EntityQuery::astNodeId == astNode.begin()->id);

    if (entity.empty()){
        return nullptr;
    }

    return entity.begin();
}

typedef boost::shared_ptr<Persistence> PersistencePtr;

BOOST_PYTHON_MODULE(persistence){
    boost::python::class_<Persistence>("Persistence", boost::python::init<ParserContext&>())
            .def("f", &Persistence::f)
            .def("persist_file", &Persistence::persistFile)
            .def("persist_variable", &Persistence::persistVariable)
            .def("persist_function", &Persistence::persistFunction)
            .def("persist_class", &Persistence::persistClass)
            .def("persist_import", &Persistence::persistImport);
}

PythonParser::PythonParser(ParserContext &ctx_) : AbstractParser(ctx_) {}

PythonParser::~PythonParser() {}

void PythonParser::markModifiedFiles() {}

bool PythonParser::cleanupDatabase() { return true; }

bool PythonParser::parse()
{
    setenv("PYTHONPATH",
           "/home/rmfcnb/ELTE/Code-Compass-Python-Plugin/:"
           "/home/rmfcnb/CodeCompass/build/plugins/python/parser/:"
           "/usr/lib/python3.8/", 1);
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