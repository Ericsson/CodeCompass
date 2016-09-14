/*
 * gitparsertest.cpp
 * 
 * This is the main file for the unit tests of the git parser's Git interface
 *
 *  Created on: Mar 2, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"

using namespace cc::parser;


TEST(GitParserRepositoryTests, OpenFail)
{
  ASSERT_THROW(
    {
      GitRepository r(GitRepository::open("___this___is___an___invalid___path___"));
    },
    GitException
  );
}

TEST(GitParserRepositoryTests, Open)
{
  GitRepository r_empty(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/empty/"));
  EXPECT_TRUE(r_empty.getInternal());
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  EXPECT_TRUE(r_simple.getInternal());
}

void initGitParserTestData()
{
  std::string gittestdir = std::string(TOP_SRCDIR) + "parser/gitparser/test/";
  std::string rmCommand = "rm -rf " + gittestdir + "/test_src/";
  std::string unzipCommand = "unzip -q " + gittestdir + "/gittestdata.zip -d " + gittestdir;         
  system(rmCommand.c_str());
  system(unzipCommand.c_str());
}

int main(int argc, char *argv[])
{
  using namespace cc::util;
  
  initGitParserTestData();
  
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
    new StandardErrorLogStrategy()));

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}