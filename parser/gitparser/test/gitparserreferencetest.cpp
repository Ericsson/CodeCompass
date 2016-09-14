/*
 * gitparserblobtest.cpp
 *
 *  Created on: Apr 20, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <util/streamlog.h>

#include "gitparser/gitrepository.h"
#include "gitparser/gitreference.h"

using namespace cc::parser;


TEST(GitParserReferenceTests, ReferenceListEmpty)
{
  GitRepository r_empty(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/empty/"));
  std::vector<std::string> refList = GitReference::getList(r_empty);
  
  EXPECT_EQ(0U, refList.size());
}
  
  
TEST(GitParserReferenceTests, ReferenceListOneRef)
{
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  std::vector<std::string> refList = GitReference::getList(r_simple);
  
  ASSERT_EQ(1U, refList.size());
  EXPECT_EQ(std::string("refs/heads/master"), refList[0]);
}


TEST(GitParserReferenceTests, RepositoryHead)
{
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  
  bool b = r_simple.isHeadDetached();
  EXPECT_FALSE(b);
  
  auto ref = r_simple.head();

  ASSERT_EQ(GitReference::GIT_REF_OID, ref->getType());

  const char *refName = ref->getName();
  ASSERT_NE(nullptr, refName);
  EXPECT_EQ(std::string("refs/heads/master"), refName);
}
