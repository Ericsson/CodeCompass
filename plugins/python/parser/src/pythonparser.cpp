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

bool PythonParser::parse()
{
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(boost::filesystem::is_directory(path))
    {
      util::iterateDirectoryRecursive(path, [this](const std::string& currPath_)
      {
        if (boost::filesystem::is_regular_file(currPath_) && accept(currPath_))
        {
            try {

              python::object refs = m_py_module.attr("parse")(currPath_);
              const int len = python::len(refs);
              for (int i = 0; i < len; i++)
              {
                python::object node = refs[i];
                uint64_t hash = python::extract<uint64_t>(node["hash"]);
              }

              if(len > 0)
              {
                model::FilePtr pyfile = _ctx.srcMgr.getFile(currPath_);
                pyfile->parseStatus = model::File::ParseStatus::PSFullyParsed;
                pyfile->type = "PY";
                _ctx.srcMgr.updateFile(*pyfile);
              }

            }catch (const python::error_already_set&)
            {
              PyErr_Print();
            }
        }

        return true;
      });
    }
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