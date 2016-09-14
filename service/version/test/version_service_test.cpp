#include <servicetest/servicetest.h>

#include "../src/versionservice.h"

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include <plugin/servicenotavailexception.h>

#include <vector>

//using namespace cc::service::core;
using namespace cc::service::version;


class VersionServiceTest : public cc::servicetest::ServiceTestBase
{
};

//If there is not repository, the constructor should throw an exception.
TEST_F(VersionServiceTest, NoRepositoryTest)
{
  bool exThrown=false;
  try{
    VersionServiceHandler srv(EmptyDatabase(this));
  }catch(ServiceNotAvailException & ex){
    exThrown=true;
  }  
  EXPECT_EQ(exThrown,true);
}

TEST_F(VersionServiceTest, GetRepositoryList)
{
  VersionServiceHandler srv(TinyXmlWithGitDatabase(this));
  
  std::vector<cc::service::version::Repository> repoList;
  srv.getRepositoryList(repoList);
  ASSERT_EQ(1U, repoList.size());
  
  const cc::service::version::Repository& repomodel = repoList.front();
  EXPECT_EQ("tinyxmlwithgit", repomodel.name);
  EXPECT_FALSE(repomodel.isHeadDetached);
  EXPECT_EQ("refs/heads/master", repomodel.head);
}

inline std::string getSingleRepoId(VersionServiceHandler& srv)
{
  std::vector<cc::service::version::Repository> repoList;
  srv.getRepositoryList(repoList);
  if (1U != repoList.size()) {
    return "";
  }
  
  return repoList.front().pathHash;
}

TEST_F(VersionServiceTest, GetCommitList)
{
  const int n = 15;
  
  VersionServiceHandler srv(TinyXmlWithGitDatabase(this));
  std::string repoId = getSingleRepoId(srv);
  ASSERT_NE("", repoId);
  
  CommitListFilteredResult commitListFilteredResult;
  srv.getCommitListFiltered(
    commitListFilteredResult,
    repoId,
    "cf33e37d25346d108ef00b3bfa447b9a8f69382f",
    n,
    0,
    ""
  );
  
  ASSERT_TRUE(commitListFilteredResult.hasRemaining);
  ASSERT_EQ(n, commitListFilteredResult.newOffset);
  ASSERT_EQ(n, (int)commitListFilteredResult.result.size());
  
}


int main(int argc, char *argv[])
{
  cc::util::StreamLog::setStrategy(std::shared_ptr<cc::util::LogStrategy>(
    new cc::util::StandardErrorLogStrategy()));
  cc::util::StreamLog::setLogLevel(cc::util::DEBUG);

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

