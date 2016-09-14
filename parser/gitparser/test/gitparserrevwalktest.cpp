/*
 * gitparserrevwalktest.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <vector>

#include <util/streamlog.h>

#include "gitparser/gitrevwalk.h"
#include "gitparser/gitrepository.h"
#include "gitparser/gitreference.h"
#include "gitparser/githexoid.h"

using namespace cc::parser;


TEST(GitParserReferenceTests, RevWalkSimple)
{
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  
  std::vector<GitOid> expected_oids{
    GitHexOid("618aefc79b79482ff80f3bf6a8a6baf103868461").toOid(),
    GitHexOid("befd3ff27f44c50c441589631b3935b06fdc3a09").toOid(),
    GitHexOid("49cf9b6d7ab307a4601e48e6a837bb329b3a14af").toOid(),
  };
  
  GitRevWalk walker(r_simple);
  walker.pushRef("refs/heads/master");
  std::pair<bool, GitOid> curr;
  int i = 0;
  while ((curr = walker.next()).first) {
    ASSERT_NE(i, (int)expected_oids.size());
    //making pairs to list the error position as well
    EXPECT_EQ(std::make_pair(i, expected_oids[i]), std::make_pair(i, curr.second));
    ++i;
  }
  EXPECT_EQ((int)expected_oids.size(), i);
}


TEST(GitParserReferenceTests, RevWalkBranches)
{
  GitRepository r_branches(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/branches/"));
  
  std::vector<GitOid> expected_oids{
    GitHexOid("2e36b87e1763fb75cbf891243b5a76c637a7989d").toOid(),
    GitHexOid("d9afbe784107970a9b1aa7059ae070ccf72a8acc").toOid(),
    GitHexOid("ee839a0a1418277103f28dbd46886030404b90d1").toOid(),
    GitHexOid("5ab9f362bedbea2200fbc727276f08adfe638a8d").toOid(),
    GitHexOid("43c667a88a9655fd3995c867c6d5b2212a3f4f27").toOid(),
    GitHexOid("abbe06cbf1862091e6881748417d35ebccf75f37").toOid(),
    GitHexOid("677972837dfb3820a3654c9619d5f35adb5a6296").toOid(),
    GitHexOid("e96b5bd95fe5dfb17b5388bc8affe6f8181e2362").toOid(),
    GitHexOid("2c8f10a9a5b3272533e4f13a7cbb078b38e77ecb").toOid(),
    GitHexOid("3762377862ab324b6ff040ad01785aaa0726303a").toOid(),
    GitHexOid("5e8d66911a7b05efa222367f379fe8ef7028ef76").toOid(),
    GitHexOid("21b4cf44fe2de7b01863a7a502dc7f8ba1e27c6c").toOid(),
  };
  
  GitRevWalk walker(r_branches);
  walker.pushRef("refs/heads/master");
  std::pair<bool, GitOid> curr;
  int i = 0;
  while ((curr = walker.next()).first) {
    ASSERT_NE(i, (int)expected_oids.size());
    //making pairs to list the error position as well
    EXPECT_EQ(std::make_pair(i, expected_oids[i]), std::make_pair(i, curr.second));
    ++i;
  }
  EXPECT_EQ((int)expected_oids.size(), i);
}
