#include <pythonparser/pythonparser.h>
#include <boost/filesystem.hpp>
#include <util/logutil.h>
#include <memory>
#include <string>

namespace cc
{
namespace parser
{

PythonParser::PythonParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  // Init Python Interpreter
  std::string py_parser_dir = _ctx.compassRoot + "/lib/parserplugin/pyparser/";
  setenv("PYTHONPATH", py_parser_dir.c_str(), 1);
  
  Py_Initialize();

  // Init PyParser module
  try {
    m_py_module = python::import("parser");

    // Set venv
    if (_ctx.options.count("venv"))
    {
      std::string venv = _ctx.options["venv"].as<std::string>();
      m_py_module.attr("venv_config")(venv);
    }
  
  }catch (const python::error_already_set&)
  {
    PyErr_Print();
  }

}

bool PythonParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".py";
}

void PythonParser::parseFile(const std::string& path_, PYNameMap& map)
{
  try {

    // Call PythonParser parse(path)
    python::object result = m_py_module.attr("parse")(path_);
    python::object nodes = result["nodes"];
    std::string status = python::extract<std::string>(result["status"]);

    const int len = python::len(nodes);
    for (int i = 0; i < len; i++)
    {
      python::object node = nodes[i];

      model::PYName pyname;
      pyname.id = python::extract<uint64_t>(node["id"]);
      pyname.ref_id = python::extract<uint64_t>(node["ref_id"]);
      pyname.full_name = python::extract<std::string>(node["full_name"]);
      pyname.is_definition = python::extract<bool>(node["is_definition"]);
      pyname.is_builtin = python::extract<bool>(node["is_builtin"]);
      pyname.line_start = python::extract<uint64_t>(node["line_start"]);
      pyname.line_end = python::extract<uint64_t>(node["line_end"]);
      pyname.column_start = python::extract<uint64_t>(node["column_start"]);
      pyname.column_end = python::extract<uint64_t>(node["column_end"]);
      pyname.file_id = python::extract<uint64_t>(node["file_id"]);
      pyname.value = python::extract<std::string>(node["value"]);
      pyname.type = python::extract<std::string>(node["type"]);

      // Put in map
      if(map.find(pyname.id) == map.end())
      {
        map[pyname.id] = pyname;
      }
    }

    if(status != "none")
    {
      model::FilePtr pyfile = _ctx.srcMgr.getFile(path_);
      
      if(status == "full")
      {
        m_parse_result.full++;
        pyfile->parseStatus = model::File::ParseStatus::PSFullyParsed;
      }else if (status == "partial")
      {
        m_parse_result.partial++;
        pyfile->parseStatus = model::File::ParseStatus::PSPartiallyParsed;
      }
      
      pyfile->type = "PY";
      _ctx.srcMgr.updateFile(*pyfile);
    }

  }catch (const python::error_already_set&)
  {
    PyErr_Print();
  }
}

bool PythonParser::parse()
{
  PYNameMap map;
  m_parse_result.full = 0;
  m_parse_result.partial = 0;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(boost::filesystem::is_directory(path))
    {
      util::iterateDirectoryRecursive(path, [&](const std::string& currPath_)
      {
        if (boost::filesystem::is_regular_file(currPath_) && accept(currPath_))
        {
          PythonParser::parseFile(currPath_, map);
        }

        return true;
      });
    }
  }

  // Insert into database
  for(const auto& [key, value] : map)
  {
    LOG(debug) << "Inserting PYName " << value.id;
    cc::util::OdbTransaction {_ctx.db} ([&]
    {
      _ctx.db->persist(value);  
    });
  }

  LOG(info) << "[PythonParser] Parsing finished!";
  LOG(info) << "[PythonParser] Fully parsed files: " << m_parse_result.full;
  LOG(info) << "[PythonParser] Partially parsed files: " << m_parse_result.partial;

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
      ("venv", po::value<std::string>(),
       "Set 'venv' to specify the project's Python virtual environment path.");

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