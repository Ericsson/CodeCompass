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
  bool accept(const std::string& path_);
  void parseFile(const std::string& path_, PYNameMap& map);
  model::PYName::PYNameType getPYNameType(const std::string& str);
  python::object m_py_module;
};
  
} // parser
} // cc

#endif // CC_PARSER_PYTHONPARSER_H
