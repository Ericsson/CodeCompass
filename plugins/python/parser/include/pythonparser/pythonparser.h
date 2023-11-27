#ifndef CC_PARSER_PYTHONPARSER_H
#define CC_PARSER_PYTHONPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>
#include <util/parserutil.h>
#include <boost/python.hpp>

namespace cc
{
namespace parser
{
  
namespace python = boost::python;

class PythonParser : public AbstractParser
{
public:
  PythonParser(ParserContext& ctx_);
  virtual ~PythonParser();
  virtual bool parse() override;
private:
  bool accept(const std::string& path_);
  python::object m_py_module;
};
  
} // parser
} // cc

#endif // CC_PARSER_PYTHONPARSER_H
