/*
 * parsertest.h
 *
 *  Created on: Jul 3, 2014
 *      Author: ezoltbo
 */

#ifndef CC_PARSER_PARSERTEST_H_
#define CC_PARSER_PARSERTEST_H_

#include <gtest/gtest.h>

#include <model/workspace.h>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <projectparser/xmlprojectparser.h>
#include <cxxparser/cxxparser.h>
#include <util/environment.h>

#include <model/cxx/cppastnode-odb.hxx>

namespace cc
{

class ParserTest : public ::testing::Test
{
protected:
  std::shared_ptr<parser::CXXParser>            _parserCXX;
  std::shared_ptr<parser::GeneralProjectParser> _parserGeneral;
  std::shared_ptr<parser::XmlProjectParser>     _parserXml;
  std::shared_ptr<model::Workspace>             _workspace;
  parser::FileParser&                           _fileParser;
  parser::Parser&                               _parser;

protected:
  ParserTest() :
    _workspace(0),
    _fileParser(parser::FileParser::instance()),
    _parser(parser::Parser::getParser())
  {
    cc::util::Environment::init();
  }

  void init(const std::string& dbname, const std::string& project)
  {
    unlink(dbname.c_str());


    _workspace = model::Workspace::getCreateWorkspace("sqlite:database="
      + dbname, model::Workspace::Create);
    auto pp = parser::ProjectParser::createParseProperties(_workspace);

    parser::SourceManager srcMgr(_workspace, pp);
    _parserCXX.reset(new parser::CXXParser(_workspace));
    _parserGeneral.reset(new parser::GeneralProjectParser(
      _workspace, pp, srcMgr));
    _parserXml.reset(new parser::XmlProjectParser(
      _workspace, pp,srcMgr));

    _fileParser.registerParser(_parserCXX);

    _parser.registerProjectParser(_parserGeneral);
    _parser.registerProjectParser(_parserXml);

    _parser.parseProject(project);
  }

  template <typename T>
  T loadEntity(const std::string& qualifiedName)
  {
    typedef odb::query<T> TQ;

    auto db = _workspace->getDb();
    auto result = db->query<T>(TQ::qualifiedName == qualifiedName);

    if (result.empty())
    {
      throw std::runtime_error("Not found entity: " + qualifiedName);
    }

    return *result.begin();
  }

  int countUsage(model::HashType hash, model::CppAstNode::AstType astType)
  {
    typedef odb::query<model::AstCountGroupByFiles> ACQ;

    auto db = _workspace->getDb();
    auto result = db->query<model::AstCountGroupByFiles>(
      ACQ::CppAstNode::mangledNameHash == hash &&
      ACQ::CppAstNode::astType == astType);

    int count = 0;

    for (auto elem : result)
    {
      count += elem.count;
    }

    return count;
  };
};

} // cc

#endif /* CC_PARSER_PARSERTEST_H_ */
