#ifndef CC_TEST_PYTHON_PYTHONPARSERFIXTURE_H_
#define CC_TEST_PYTHON_PYTHONPARSERFIXTURE_H_

#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <iostream>

#include <gtest/gtest.h>

#include <model/workspace.h>

#include <parser/parser.h>
#include <pythonparser/pythonparser.h>
#include <parser/projectparser.h>
#include <projectparser/generalprojectparser.h>
#include <projectparser/xmlprojectparser.h>
#include <cxxparser/cxxparser.h>

#include <model/cxx/cppastnode-odb.hxx>

#include <model/python/pythonastnode.h>
#include <model/python/pythonastnode-odb.hxx>

#include <model/python/pythonbinding.h>
#include <model/python/pythonbinding-odb.hxx>

#include <util/streamlog.h>


namespace cc
{
namespace test
{
namespace python
{


class PythonFixture : public ::testing::Test
{
protected:
  std::shared_ptr<parser::PythonParser>         _parserPython;
  std::shared_ptr<model::Workspace>             _workspace;
  parser::Parser&                               _parser;
  std::shared_ptr<parser::GeneralProjectParser> _parserGeneral;

protected:
  PythonFixture()
    : _workspace(0)
    , _parser(parser::Parser::getParser())
  {
  }

  void init(const std::string & dbname, const std::string & project)
  {
    unlink(dbname.c_str());

    _workspace = model::Workspace::getCreateWorkspace("sqlite:database="
      + dbname, model::Workspace::Create);

    auto pp = parser::ProjectParser::createParseProperties(_workspace);

    parser::SourceManager srcMgr(_workspace, pp);
    _parserGeneral.reset(new parser::GeneralProjectParser(
      _workspace, pp, srcMgr));
    _parserPython.reset(new cc::parser::PythonParser(_workspace));

    cc::parser::ProjectParser::registerTraversal(_parserPython);
    _parser.registerProjectParser(_parserGeneral);

    _parser.parseProject(project);

  }

  template <typename T>
  T loadEntity(const std::string& name_)
  {
    typedef odb::query<T> TQ;

    auto db = _workspace->getDb();
    auto result = db->query<T>(TQ::name == name_);

    if (result.empty())
      throw std::runtime_error("Not found entity: " + name_);

    return *result.begin();
  }

  cc::model::PythonAstNode loadEntity(
    const cc::model::Position & startPos_,
    const std::string & filename_)
  {
    typedef odb::query<cc::model::PythonAstNode> AstNodeQ;

    auto db = _workspace->getDb();
    auto result = db->query<cc::model::PythonAstNode>(
      AstNodeQ::location.range.start.line == startPos_.line
      &&
      AstNodeQ::location.range.start.column == startPos_.column
      //TODO
      //&&
      //AstNodeQ::location.file->filename = filename_
      );

    if (result.empty())
      throw std::runtime_error("Not found entity at: "
        + filename_
        + ":" + std::to_string(startPos_.line)
        + ":" + std::to_string(startPos_.column));

    return *result.begin();
  }
};

} // python
} // test
} // cc

#endif // CC_TEST_PYTHON_PYTHONPARSERFIXTURE_H_
