/*
 * gitparsertreetest.cpp
 *
 *  Created on: Mar 2, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"
#include "gitparser/gitoid.h"
#include "gitparser/githexoid.h"
#include "gitparser/gitblob.h"
#include "gitparser/gittree.h"

using namespace cc::parser;


TEST(GitParserTreeTests, LookUpFail)
{
  GitOid oid;
  
  GitRepository r_empty(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/empty/"));
  ASSERT_THROW(
    {
      GitTree tree(GitTree::lookUp(r_empty, oid));
    },
    GitException
  );

  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  ASSERT_THROW(
    {
      GitTree tree(GitTree::lookUp(r_simple, oid));
    },
    GitException
  );  
}

TEST(GitParserTreeTests, LookUp)
{
  GitOid oid = GitHexOid("ae79338a6998cff58ffd2f87542352b5e6e55edc").toOid();
  
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  GitTree tree = GitTree::lookUp(r_simple, oid);
  
  //list the tree and check if tree objects are loadable
  std::vector<std::string> expectedFiles{
    ".gitignore",
    "Makefile",
    "hello.cpp"
  };
  
  const size_t N = tree.getEntryCount();
  ASSERT_EQ(3U, N);
  for (size_t i = 0; i < N; ++i)
  {
    GitTreeEntry e = tree.getEntryByIndex(i);
    EXPECT_EQ(expectedFiles[i], e.getName());
    EXPECT_EQ(GitTreeEntry::GIT_FILEMODE_BLOB, e.getMode());
    GitBlob b = GitBlob::lookUp(r_simple, e.getPointedId());
  } 
}

//TODO test trees with subtrees
