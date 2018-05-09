#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <algorithm>

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
    const std::map<std::string, std::string>& expectedValues_,
    const std::vector<std::string>& ignoredValues_ = {},
    const std::vector<std::string>& optionalProperties_ = {})
  {
    std::map<std::string, std::string> props;

    AstNodeInfo anFrom = _helper.getAstNodeInfoByPos(line_, col_, fid_);

    _transaction([&, this]()
    {
      _cppservice->getProperties(props, anFrom.id);
    });

    if (props.size() < expectedValues_.size() + ignoredValues_.size())
      LOG(debug) << "Position: " << line_ << ":" << col_;

    EXPECT_LE(expectedValues_.size() + ignoredValues_.size(),
              props.size());
    for (const auto& prop : props)
    {
      if (std::find(optionalProperties_.begin(), optionalProperties_.end(),
                    prop.first) != optionalProperties_.end())
      {
        // Certain properties may or may not exist in the result set. Ignore
        // if they are found.
        LOG(debug) << "AstNode property " << prop.first << " optional."
          << " Current value is: " << prop.second << ".";
        continue;
      }

      if (std::find(ignoredValues_.begin(), ignoredValues_.end(), prop.first)
          != ignoredValues_.end())
      {
        // If a property is found to not have its value tested but only the
        // existence of it, accept it.
        LOG(debug) << "AstNode property " << prop.first << " value ignored."
          << " Current value is: " << prop.second << ".";
        continue;
      }

      if (expectedValues_.find(prop.first) == expectedValues_.end())
      {
        LOG(error)
          << "AstNode properties " << prop.first
          << " doesn't match. There wasn't an expected value."
          << " Current value is: " << prop.second << "!";
        FAIL() << "Got value which isn't ignored, but wasn't expecting "
                  "anything ";
      }

      if (expectedValues_.at(prop.first) != prop.second)
        // Otherwise, check if the value is the value we expected to get.
        LOG(error)
          << "AstNode properties " << prop.first
          << " doesn't match. Expected value is: "
          << expectedValues_.at(prop.first) << ". Current value is: "
          << prop.second << "!";
      EXPECT_EQ(expectedValues_.at(prop.first), prop.second);
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

    checkProperties(_simpleClassHeader, 9, 10, expected,
                    {"Size"}, {"Alignment"});
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "NestedClass"},
      {"Qualified name", "cc::test::NestedClass"},
      {"POD type", "true"}
    };

    checkProperties(_nestedClassHeader, 9, 10, expected,
                    {"Size"}, {"Alignment"});
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "InnerClass"},
      {"Qualified name", "cc::test::NestedClass::InnerClass"},
      {"POD type", "true"}
    };

    checkProperties(_nestedClassHeader, 13, 15, expected,
                    {"Size"}, {"Alignment"});
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

    checkProperties(_inheritanceClassHeader, 9, 10, expected,
                    {"Size"}, {"Alignment"});
  }

  {
    std::map<std::string, std::string> expected =
    {
      {"Name", "DerivedClass"},
      {"Qualified name", "cc::test::DerivedClass"}
    };

    checkProperties(_inheritanceClassHeader, 31, 15, expected,
                    {"Size"}, {"Alignment"});
  }
}
