/*
 * gitparsercommittest.cpp
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
#include "gitparser/gittree.h"
#include "gitparser/gitcommit.h"

using namespace cc::parser;


TEST(GitParserCommitTests, LookUpFail)
{
  GitOid oid;
  
  GitRepository r_empty(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/empty/"));
  ASSERT_THROW(
    {
      GitCommit c(GitCommit::lookUp(r_empty, oid));
    },
    GitException
  );

  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  ASSERT_THROW(
    {
      GitCommit c(GitCommit::lookUp(r_simple, oid));
    },
    GitException
  );  
}

TEST(GitParserCommitTests, LookUp)
{
  GitOid oid = GitHexOid("618aefc79b79482ff80f3bf6a8a6baf103868461").toOid();
  
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  GitCommit c = GitCommit::lookUp(r_simple, oid);

  std::string authorName = "Tamás Cséri";
  std::string authorEmail = "cseri@caesar.elte.hu";
  auto author = c.getAuthor();
  EXPECT_EQ(authorName, author.getName());
  EXPECT_EQ(authorEmail, author.getEmail());
  auto committer = c.getCommitter();
  EXPECT_EQ(authorName, committer.getName());
  EXPECT_EQ(authorEmail, committer.getEmail());

  GitTree tree_a = GitTree::lookUp(r_simple, c.getTreeId());
  GitTree tree_b = c.getTree();
  
  EXPECT_EQ(tree_a.getId(), tree_b.getId());
}

