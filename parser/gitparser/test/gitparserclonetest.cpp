/*
 * gitparserblobtest.cpp
 *
 *  Created on: May 17, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <util/streamlog.h>

#include <boost/filesystem.hpp>

#include "gitparser/gitrepository.h"
#include "gitparser/gitoid.h"
#include "gitparser/githexoid.h"
#include "gitparser/gitblob.h"
#include "gitparser/gitclone.h"

using namespace cc::parser;

namespace {

void testByReadingBlob(GitRepository & repo)
{
  GitOid oid = GitHexOid("b38652672d5159c46bc8fb08cfb976fceeef6786").toOid();
  
  GitBlob b = GitBlob::lookUp(repo, oid);
  
  ASSERT_EQ(92U, b.getDataSize());

  std::string data = b.getDataAsString();
  ASSERT_EQ(92U, data.size());
  ASSERT_EQ(56U, data.find("World"));
}

const std::string gitTestPath = "test/tmp_gittest/";
  
}

TEST(GitParserCloneTests, Clone)
{
  std::string currentTestRepoPath(gitTestPath + "clone_test");
  uint32_t x;
  boost::filesystem::remove_all(currentTestRepoPath);
  
  {
    GitClone clone;
    GitRepository repo = clone.clone(
      TOP_SRCDIR "parser/gitparser/test/test_src/simple/",
      currentTestRepoPath.c_str());
    
    EXPECT_FALSE(repo.isBare());
    testByReadingBlob(repo);
  }

  {
    GitRepository repo = GitRepository::open(currentTestRepoPath.c_str());
    
    EXPECT_FALSE(repo.isBare());
    testByReadingBlob(repo);
  }
}
  
TEST(GitParserCloneTests, CloneBare)
{
  std::string currentTestRepoPath(gitTestPath + "clonebare_test");
  uint32_t x;
  boost::filesystem::remove_all(currentTestRepoPath);
  
  {
    GitClone clone;
    clone.setBare(true);
    GitRepository repo = clone.clone(
      TOP_SRCDIR "parser/gitparser/test/test_src/simple/",
      currentTestRepoPath.c_str());

    EXPECT_TRUE(repo.isBare());
    testByReadingBlob(repo);
  }
  
  {
    GitRepository repo = GitRepository::open(currentTestRepoPath.c_str());
    
    EXPECT_TRUE(repo.isBare());
    testByReadingBlob(repo);
  }
}

