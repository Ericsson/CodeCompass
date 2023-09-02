#include <iostream>
#include <Python.h>

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/optional.hpp>

#include <pythonparser/pythonparser.h>

#include <pythonparser/pythonpersistence.h>

#include <util/logutil.h>

namespace cc {
namespace parser{

BOOST_PYTHON_MODULE(persistence)
{
    boost::python::class_<PythonPersistence>("Persistence", boost::python::init<ParserContext&>())
            .def("print", &PythonPersistence::cppprint)
            .def("log_info", &PythonPersistence::logInfo)
            .def("log_warning", &PythonPersistence::logWarning)
            .def("log_error", &PythonPersistence::logError)
            .def("log_debug", &PythonPersistence::logDebug)
            .def("persist_file", &PythonPersistence::persistFile)
            .def("persist_variable", &PythonPersistence::persistVariable)
            .def("persist_function", &PythonPersistence::persistFunction)
            .def("persist_preprocessed_class", &PythonPersistence::persistPreprocessedClass)
            .def("persist_class", &PythonPersistence::persistClass)
            .def("persist_import", &PythonPersistence::persistImport);
}

PythonParser::PythonParser(ParserContext &ctx_) : AbstractParser(ctx_) {}

PythonParser::~PythonParser()
{
}

void PythonParser::markModifiedFiles() {}

bool PythonParser::cleanupDatabase() { return true; }

bool PythonParser::parse()
{
    const std::string PARSER_SCRIPTS_DIR = _ctx.compassRoot + "/lib/parserplugin/scripts/python";
    if (!boost::filesystem::exists(PARSER_SCRIPTS_DIR) || !boost::filesystem::is_directory(PARSER_SCRIPTS_DIR))
    {
        throw std::runtime_error(PARSER_SCRIPTS_DIR + " is not a directory!");
    }

    setenv("PYTHONPATH", PARSER_SCRIPTS_DIR.c_str(), 1);

    try{
        Py_Initialize();
        init_module_persistence();

        boost::python::object module = boost::python::import("cc_python_parser.python_parser");

        if(!module.is_none())
        {
            boost::python::object func = module.attr("parse");

            if(!func.is_none() && PyCallable_Check(func.ptr()))
            {
                std::string source_path;
                for (const std::string& input : _ctx.options["input"].as<std::vector<std::string>>())
                {
                    if (boost::filesystem::is_directory(input))
                    {
                        source_path = input;
                    }
                }
                if(source_path.empty())
                {
                    LOG(error) << "No source path was found";
                } else {
                    PythonPersistencePtr persistencePtr(new PythonPersistence(_ctx));

                    func(source_path, boost::python::ptr(persistencePtr.get()));
                }
            } else {
                LOG(error) << "Cannot find function";
            }
        } else {
            LOG(error) << "Cannot import module";
        }
    } catch (boost::python::error_already_set)
    {
        PyErr_Print();
    }

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
