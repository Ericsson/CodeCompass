#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <service/cppservice.h>

#include <util/dbutil.h>

#include "servicehelper.h"

using namespace cc;
using namespace cc::service;
using namespace cc::service::test;

class CppReferenceServiceTest : public ::testing::Test
{
public:
  CppReferenceServiceTest() :
    _db(cc::util::createDatabase(dbConnectionString)),
    _transaction(_db),
    _cppservice( new CppServiceHandler(
      _db,
      std::make_shared<std::string>(""),
      boost::program_options::variables_map())),
    _helper(_db, _cppservice)
  {
    _simpleClassHeader = _helper.getFileId("simpleclass.h");
    _simpleClassSrc = _helper.getFileId("simpleclass.cpp");

    _nestedClassHeader = _helper.getFileId("nestedclass.h");
    _nestedClassSrc = _helper.getFileId("nestedclass.cpp");

    _inheritanceClassHeader = _helper.getFileId("inheritance.h");
    _inheritanceClassSrc = _helper.getFileId("inheritance.cpp");
  }

  /**
   * This function checks the definition of a clicked node. If the definition
   * size isn't equal with the expected line or any expected line number isn't
   * equal with the correct definition number, the test will fail.
   * @param line_ Clicked position line.
   * @param col_ Clicked position column.
   * @param fileId_ File id
   * @param expectedLines_ Expected lines of definitions.
   */
  void inline checkDefinition(int line_, int col_, model::FileId fileId_,
    std::vector<int> expectedLines_)
  {
    _helper.checkReferences(line_, col_, fileId_,
      { {"Definition", expectedLines_} });
  }

protected:
  std::shared_ptr<odb::database> _db;
  cc::util::OdbTransaction _transaction;
  std::shared_ptr<CppServiceHandler> _cppservice;
  ServiceHelper _helper;

  model::FileId _simpleClassHeader;
  model::FileId _simpleClassSrc;

  model::FileId _nestedClassHeader;
  model::FileId _nestedClassSrc;

  model::FileId _inheritanceClassHeader;
  model::FileId _inheritanceClassSrc;
};

/******************************************************************************
 *                            Type references
 ******************************************************************************/

//--- SimpleClass ---//

TEST_F(CppReferenceServiceTest, SimpleClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Definition", {9}},
    {"Data member", {22, 25}}, /*!< _locProtX, _locPrivX */
    {"Method", {-1, 13, 15, 17, 19}}, /*!< Copy Ctor, SimpleClass, ~SimpleClass,
                                       getPrivX, setPrivX */
  };

  _helper.checkReferences(9,  13, _simpleClassHeader, expected);
  _helper.checkReferences(8,  5,  _simpleClassSrc, expected);
}

//--- NestedClass ---//

TEST_F(CppReferenceServiceTest, NestedClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Definition", {9}},
    {"Method", {21}}, /*!< createNestedClass */
    {"Usage", {9, 8, 10, 14, 14}}
  };

  _helper.checkReferences(9,  13, _nestedClassHeader, expected);
  _helper.checkReferences(10, 10, _nestedClassSrc, expected);
  _helper.checkReferences(14, 5,  _nestedClassSrc, expected);
  _helper.checkReferences(14, 30, _nestedClassSrc, expected);
}

//--- InnerClass ---//

TEST_F(CppReferenceServiceTest, NestedInnerClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {15, 16}},
    {"Declaration", {11}},
    {"Definition", {13}},
    {"Method", {-1, -1, -1, -1, 17}}, /*!< ctor, copy ctor, move ctor,
                                           dtor, f */
  };

  _helper.checkReferences(13,  13, _nestedClassHeader, expected);
  _helper.checkReferences(10,  25, _nestedClassSrc, expected);
  _helper.checkReferences(14,  20, _nestedClassSrc, expected);
}

//--- List ---//

TEST_F(CppReferenceServiceTest, ListClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {33, 34}}, /*!< head, tail */
    {"Definition", {24}},
    {"Usage", {24}}
  };

  _helper.checkReferences(24, 10, _nestedClassHeader, expected);
}

//--- Node ---//

TEST_F(CppReferenceServiceTest, NodeInListClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {28, 29, 30}}, /*!< data, next, prev */
    {"Definition", {26}}
  };

  _helper.checkReferences(26, 12, _nestedClassHeader, expected);
}

//--- BaseClass1 ---//

TEST_F(CppReferenceServiceTest, BaseClass1Test)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {15, 18}}, /*!< _pubBaseX, _protBaseX */
    {"Definition", {9}},
    {"Inherited by", {31}}, /*!< DerivedClass */
    {"Method", {-1, -1, 11, 12, 13}}, /*!< copy ctor, operator=, ~BaseClass1,
                                           f, g */
  };

  _helper.checkReferences(9,  13, _inheritanceClassHeader, expected);
  _helper.checkReferences(31, 35, _inheritanceClassHeader, expected);
}

//--- BaseClass2 ---//

TEST_F(CppReferenceServiceTest, BaseClass2Test)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {28}}, /*!< _protBaseY */
    {"Definition", {21}},
    {"Inherited by", {31}}, /*!< DerivedClass */
    {"Method", {-1, -1, 23, 24, 25}}, /*!< copy ctor, operator=, ~BaseClass2,
                                           f, h */
  };

  _helper.checkReferences(21, 13, _inheritanceClassHeader, expected);
}

//--- DerivedClass ---//

TEST_F(CppReferenceServiceTest, DerivedClassTest)
{
  ServiceHelper::ExpectedValues expected = {
    {"Data member", {34}}, /*!< Z */
    {"Definition", {31}},
    {"Inherits from", {9, 21}}, /*!< BaseClass1, BaseClass2 */
    {"Method", {-1, -1, -1, -1, 36, 38, 40}}, /*!< f, g, h, copy ctor, move ctor
      cpy operator=, move operator=, dtor */
    {"Usage", {31, 8, 40, 45, 50}}
  };

  _helper.checkReferences(31, 13, _inheritanceClassHeader, expected);
  _helper.checkReferences(8,  15, _inheritanceClassSrc, expected);
  _helper.checkReferences(50, 10, _inheritanceClassSrc, expected);
}
