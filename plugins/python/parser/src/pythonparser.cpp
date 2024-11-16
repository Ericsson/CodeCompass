#include <pythonparser/pythonparser.h>
#include <boost/filesystem.hpp>
#include <util/logutil.h>

namespace cc
{
namespace parser
{

PythonParser::PythonParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  // Init Python Interpreter
  std::string py_parser_dir = _ctx.compassRoot + "/lib/pythonplugin/pyparser";
  std::string py_venv_dir = _ctx.compassRoot + "/lib/pythonplugin/venv";
  std::string path_env = py_venv_dir + "/bin" + ":" + getenv("PATH");

  // Set Python module path
  setenv("PYTHONPATH", py_parser_dir.c_str(), 1);

  // Activate Python venv
  setenv("VIRTUAL_ENV", py_venv_dir.c_str(), 1);
  setenv("PATH", path_env.c_str(), 1);

  Py_Initialize();

  // Init PyParser module
  try {
    m_py_module = python::import("parser");
  }
  catch (const python::error_already_set&)
  {
    PyErr_Print();
  }
}

void PythonParser::parseProject(const std::string& root_path)
{
  PYNameMap map;
  ParseResultStats parse_result;
  parse_result.full = 0;
  parse_result.partial = 0;

  try {
    python::list sys_path;
    int n_proc = _ctx.options["jobs"].as<int>();

    if(_ctx.options.count("syspath"))
    {
      std::vector<std::string> vec = _ctx.options["syspath"].as<std::vector<std::string>>();
      for(const std::string& s : vec)
      {
        sys_path.append(s);
      }
    }

    python::dict settings;
    settings["root_path"] = root_path;
    settings["sys_path"] = sys_path;
    settings["venv_path"] = (_ctx.options.count("venvpath")) ? _ctx.options["venvpath"].as<std::string>() : "";
    settings["debug"] = (bool)(_ctx.options.count("debug"));
    settings["stack_trace"] = (bool)(_ctx.options.count("stack-trace"));
    settings["type_hint"] = (bool)(_ctx.options.count("type-hint"));
    settings["submodule_discovery"] = !(_ctx.options.count("disable-submodule-discovery"));
    settings["ast"] = !(_ctx.options.count("disable-ast"));
    settings["ast_function_call"] = !(_ctx.options.count("disable-ast-function-call"));
    settings["ast_import"] = !(_ctx.options.count("disable-ast-import"));
    settings["ast_annotations"] = !(_ctx.options.count("disable-ast-annotations"));
    settings["ast_inheritance"] = !(_ctx.options.count("disable-ast-inheritance"));
    settings["ast_function_signature"] = !(_ctx.options.count("disable-ast-function-signature"));

    python::object result_list = m_py_module.attr("parseProject")(settings, n_proc);

    for(int i = 0; i < python::len(result_list); i++)
    {
      PythonParser::processFile(result_list[i], map, parse_result);
    }

  
  }catch (const python::error_already_set&)
  {
    PyErr_Print();
  }

  // Insert into database
  LOG(info) << "[pythonparser] Inserting PYNames to database...";
  cc::util::OdbTransaction {_ctx.db} ([&]
  {
    for(const auto& e : map)
    {
      _ctx.db->persist(e.second);
    }
  });

  LOG(info) << "[pythonparser] Parsing finished!";
  LOG(info) << "[pythonparser] Inserted rows: " << map.size();
  LOG(info) << "[pythonparser] Fully parsed files: " << parse_result.full;
  LOG(info) << "[pythonparser] Partially parsed files: " << parse_result.partial;
}

bool PythonParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".py";
}

void PythonParser::processFile(const python::object& obj, PYNameMap& map, ParseResultStats& parse_result)
{
  try {
    python::object nodes = obj.attr("nodes");
    const std::string status = python::extract<std::string>(obj.attr("status"));
    const std::string path = python::extract<std::string>(obj.attr("path"));

    const int len = python::len(nodes);
    for (int i = 0; i < len; i++)
    {
      python::object node = nodes[i];

      model::PYName pyname;
      pyname.id = python::extract<uint64_t>(node.attr("id"));
      pyname.ref_id = python::extract<uint64_t>(node.attr("ref_id"));
      pyname.parent = python::extract<uint64_t>(node.attr("parent"));
      pyname.parent_function = python::extract<uint64_t>(node.attr("parent_function"));
      pyname.full_name = python::extract<std::string>(node.attr("full_name"));
      pyname.is_definition = python::extract<bool>(node.attr("is_definition"));
      pyname.is_import = python::extract<bool>(node.attr("is_import"));
      pyname.is_builtin = python::extract<bool>(node.attr("is_builtin"));
      pyname.line_start = python::extract<uint64_t>(node.attr("line_start"));
      pyname.line_end = python::extract<uint64_t>(node.attr("line_end"));
      pyname.column_start = python::extract<uint64_t>(node.attr("column_start"));
      pyname.column_end = python::extract<uint64_t>(node.attr("column_end"));
      pyname.file_id = python::extract<uint64_t>(node.attr("file_id"));
      pyname.value = python::extract<std::string>(node.attr("value"));
      pyname.type = python::extract<std::string>(node.attr("type"));
      pyname.type_hint = python::extract<std::string>(node.attr("type_hint"));
      pyname.is_call = python::extract<bool>(node.attr("is_call"));

      // Put in map
      if(map.find(pyname.id) == map.end())
      {
        map[pyname.id] = pyname;
      }
    }

    if(status != "none")
    {
      model::FilePtr pyfile = _ctx.srcMgr.getFile(path);
      
      if(status == "full")
      {
        parse_result.full++;
        pyfile->parseStatus = model::File::ParseStatus::PSFullyParsed;
      }else if (status == "partial")
      {
        parse_result.partial++;
        pyfile->parseStatus = model::File::ParseStatus::PSPartiallyParsed;
      }
      
      pyfile->type = "PY";
      _ctx.srcMgr.updateFile(*pyfile);
    }

    // Additional paths (example: builtin definition paths)
    // These files need to be added to db
    python::object imports = obj.attr("imports");
    for (int i = 0; i < python::len(imports); i++)
    {
      std::string p = python::extract<std::string>(imports[i]);

      model::FilePtr file = _ctx.srcMgr.getFile(p);
      file->type = "PY";
      _ctx.srcMgr.updateFile(*file);
    }

  }catch (const python::error_already_set&)
  {
    PyErr_Print();
  }
}

bool PythonParser::parse()
{
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    PythonParser::parseProject(path);
  }

  return true;
}

PythonParser::~PythonParser()
{
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Python Plugin");
    description.add_options()
      ("venvpath", po::value<std::string>(),
        "Set 'venvpath' to specify the project's Python virtual environment path.")
      ("syspath", po::value<std::vector<std::string>>(),
        "Additional sys path for the parser.")
      ("stack-trace",
        "Enable error stack trace.")
      ("type-hint",
        "Enable type hint parsing.")
      ("disable-submodule-discovery",
        "Disable submodule disovery. Submodules will not be added to sys path.")
      ("disable-ast",
        "Disable all AST parsing modules.")
      ("disable-ast-function-call",
        "Disable AST function call parsing.")
      ("disable-ast-import",
        "Disable AST import parsing.")
      ("disable-ast-annotations",
        "Disable AST annotation parsing.")
      ("disable-ast-inheritance",
        "Disable AST inheritance parsing.")
      ("disable-ast-function-signature",
        "Disable AST function signature parsing.")
      ("debug",
        "Enable parsing in debug mode.");

    return description;
  }

  std::shared_ptr<PythonParser> make(ParserContext& ctx_)
  {
    return std::make_shared<PythonParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
