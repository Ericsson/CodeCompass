#ifndef CC_PARSER_PYTHONPARSER_H
#define CC_PARSER_PYTHONPARSER_H

#include <string>
#include <map>
#include <parser/abstractparser.h>
#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>
#include <util/parserutil.h>
#include <boost/python.hpp>
#include <model/pyname.h>
#include <model/pyname-odb.hxx>
namespace cc
{
namespace parser
{
  
namespace python = boost::python;

typedef std::unordered_map<std::uint64_t, model::PYName> PYNameMap;

class PythonParser : public AbstractParser
{
public:
  PythonParser(ParserContext& ctx_);
  virtual ~PythonParser();
  virtual bool parse() override;
private:
  struct ParseResult {
    std::uint32_t partial;
    std::uint32_t full;
  };

  bool accept(const std::string& path_);
  void parseFile(const std::string& path_, PYNameMap& map);
  python::object m_py_module;
  ParseResult m_parse_result;
  void prepareInput(const std::string& root_path);
};
  
} // parser
} // cc

#endif // CC_PARSER_PYTHONPARSER_H
