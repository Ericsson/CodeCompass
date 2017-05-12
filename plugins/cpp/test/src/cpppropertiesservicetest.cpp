#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <service/cppservice.h>

#include <util/dbutil.h>
#include <util/logutil.h>

#include "servicehelper.h"

using namespace cc;

class CppPropertiesServiceTest : public ::testing::Test
{
public:
  CppPropertiesServiceTest() :
    _db(cc::util::createDatabase(dbConnectionString)),
    _transaction(_db),
    _cppservice(new CppServiceHandler(
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

  void checkProperties(model::FileId fid_, int line_, int col_,
    std::map<std::string, std::string>& expectedLines_)
  {
    std::map<std::string, std::string> props;

    AstNodeInfo anFrom = _helper.getAstNodeInfoByPos(line_, col_, fid_);

    _transaction([&, this]()
    {
      _cppservice->getProperties(props, anFrom.id);
    });

    if (props.size() < expectedLines_.size())
      LOG(debug) << "Position: " << line_ << ":" << col_;

    EXPECT_LE(expectedLines_.size(), props.size());
    for (const auto& prop : props)
    {
      if (expectedLines_[prop.first] != prop.second)
        LOG(error)
          << "AstNode properties " << prop.first
          << " doesn't match. Expected value is: " << expectedLines_[prop.first]
          << ". Current value is: " << prop.second << "!";

      EXPECT_EQ(expectedLines_[prop.first], prop.second);
    }
  }

protected:
  std::shared_ptr<odb::database> _db;
  cc::util::OdbTransaction _transaction;
  std::shared_ptr<CppServiceHandler> _cppservice;
  cc::service::test::ServiceHelper _helper;

  model::FileId _simpleClassHeader;
  model::FileId _simpleClassSrc;

  model::FileId _nestedClassHeader;
  model::FileId _nestedClassSrc;

  model::FileId _inheritanceClassHeader;
  model::FileId _inheritanceClassSrc;

};

TEST_F(CppPropertiesServiceTest, ClassPropertiesTest)
{
  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "SimpleClass"},
      {"Qualified name", "cc::test::SimpleClass"}
    };

    checkProperties(_simpleClassHeader, 9, 10, expected);
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "NestedClass"},
      {"Qualified name", "cc::test::NestedClass"},
      {"POD type", "true"}
    };

    checkProperties(_nestedClassHeader, 9, 10, expected);
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "InnerClass"},
      {"Qualified name", "cc::test::NestedClass::InnerClass"},
      {"POD type", "true"}
    };

    checkProperties(_nestedClassHeader, 13, 15, expected);
  }
}

TEST_F(CppPropertiesServiceTest, InheritancePropertiesTest)
{
  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "BaseClass1"},
      {"Qualified name", "cc::test::BaseClass1"},
      {"Abstract type", "true"}
    };

    checkProperties(_inheritanceClassHeader, 9, 10, expected);
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "DerivedClass"},
      {"Qualified name", "cc::test::DerivedClass"}
    };

    checkProperties(_inheritanceClassHeader, 31, 15, expected);
  }
}
