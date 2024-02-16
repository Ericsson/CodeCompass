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
  LOG(info) << "py_parser_dir: " << py_parser_dir;
  setenv("PYTHONPATH", py_parser_dir.c_str(), 1);
  
  Py_Initialize();

  // Init PyParser module
  try {
    m_py_module = python::import("parser");
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

model::PYName::PYNameType PythonParser::getPYNameType(const std::string& str)
{
  static std::unordered_map<std::string,model::PYName::PYNameType> const table = { 
    {"class",model::PYName::PYNameType::Class},
    {"function",model::PYName::PYNameType::Function},
    {"instance",model::PYName::PYNameType::Instance},
    {"keyword",model::PYName::PYNameType::Keyword},
    {"module",model::PYName::PYNameType::Module},
    {"param",model::PYName::PYNameType::Param},
    {"path",model::PYName::PYNameType::Path},
    {"property",model::PYName::PYNameType::Property},
    {"statement",model::PYName::PYNameType::Statement},
    {"unknown",model::PYName::PYNameType::Unknown}
  };

  auto it = table.find(str);
  if (it != table.end()) {
    return it->second;
  } else {
    return model::PYName::PYNameType::Unknown;
  }
}

void PythonParser::parseFile(const std::string& path_, PYNameMap& map)
{
  try {

    python::object result = m_py_module.attr("parse")(path_);
    python::object nodes = result["nodes"];
    std::string status = python::extract<std::string>(result["status"]);

    const int len = python::len(nodes);
    for (int i = 0; i < len; i++)
    {
      python::object node = nodes[i];

      model::PYName pyname;
      pyname.id = python::extract<uint64_t>(node["id"]);
      pyname.refid = python::extract<uint64_t>(node["refid"]);
      pyname.full_name = python::extract<std::string>(node["full_name"]);
      pyname.is_definition = python::extract<bool>(node["is_definition"]);
      pyname.is_builtin = python::extract<bool>(node["is_builtin"]);
      pyname.line = python::extract<uint64_t>(node["line"]);
      pyname.column = python::extract<uint64_t>(node["column"]);
      pyname.line_start = python::extract<uint64_t>(node["line_start"]);
      pyname.line_end = python::extract<uint64_t>(node["line_end"]);
      pyname.column_start = python::extract<uint64_t>(node["column_start"]);
      pyname.column_end = python::extract<uint64_t>(node["column_end"]);

      // PYNameType
      std::string type_str = python::extract<std::string>(node["type"]);
      pyname.type = PythonParser::getPYNameType(type_str);

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
        pyfile->parseStatus = model::File::ParseStatus::PSFullyParsed;
      }else if (status == "partial")
      {
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