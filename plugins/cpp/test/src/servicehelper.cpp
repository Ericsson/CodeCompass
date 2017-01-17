#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <util/dbutil.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>

#include "servicehelper.h"

using namespace cc::service::language;

namespace cc
{
namespace service
{
namespace test
{

ServiceHelper::ServiceHelper(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<CppServiceHandler> cppservice_) :
    _db(db_), _transaction(_db), _cppservice(cppservice_)
{
}

model::FileId ServiceHelper::getFileId(const std::string& fileName_)
{
  return _transaction([&, this](){
    typedef odb::query<model::File>  FQuery;

    odb::result<model::File> res =
      _db->query<model::File>( FQuery::filename == fileName_);

    return (*res.begin()).id;
  });
}

AstNodeInfo ServiceHelper::getAstNodeInfoByPos(
  int line_, int col_, model::FileId fid_)
{
  AstNodeInfo nodeInfo;

  core::FilePosition fp;

  fp.file = std::to_string(fid_);
  fp.pos.line = line_;
  fp.pos.column = col_;

  _transaction([&, this](){
    _cppservice->getAstNodeInfoByPosition(nodeInfo, fp);
  });

  return nodeInfo;
}

std::map<std::string, std::int32_t> ServiceHelper::getReferenceType(
  core::AstNodeId nodeId_)
{
  std::map<std::string, std::int32_t> refTypes;

  _transaction([&, this]()
  {
    _cppservice->getReferenceTypes(refTypes, nodeId_);
  });

  return refTypes;
}

void ServiceHelper::checkReferences(
  int line_, int col_,
  const model::FileId& fid_,
  const ExpectedValues& expectedValues_)
{
  AstNodeInfo anFrom = getAstNodeInfoByPos(line_, col_, fid_);
  core::FileId fileId = std::to_string(fid_);
  std::map<std::string, std::int32_t> refType = getReferenceType(anFrom.id);

  for(const auto& exp: expectedValues_)
  {
    if(refType.count(exp.first))
    {
      std::string expectedType = exp.first;
      std::vector<int> expectedValues = exp.second;

      std::vector<AstNodeInfo> references;
      _transaction([&, this]()
      {
        _cppservice->getReferences(
          references, anFrom.id, refType[expectedType], {});
      });

      if (expectedValues.size() != references.size())
        LOG(error)
          << "\nReference: " << exp.first
          << "Position: " << line_ << ":" << col_;

      EXPECT_EQ(expectedValues.size(), references.size());

      for (const auto& ref : references)
      {
        bool contain = std::find(expectedValues.begin(), expectedValues.end(),
          ref.range.range.startpos.line) != expectedValues.end();

        if(!contain)
          LOG(error)
            << "\nReference: " << exp.first
            << "\nAstNodeValue: " << ref.astNodeValue
            << "\nExpected line: " << ref.range.range.startpos.line;

        EXPECT_TRUE(contain);
      }
    }
  }
}

} // test
} // service
} // cc
