/*
 * gitparserblametest.cpp
 *
 *  Created on: Jun 12, 2014
 *      Author: cseri
 */

#include <vector>
#include <fstream>

#include <gtest/gtest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include "gitparser/gitrepository.h"
#include "gitparser/gitblame.h"
#include "gitparser/githexoid.h"

using namespace std;
using namespace cc::parser;


TEST(GitParserBlameTests, BlameFile)
{
  //these are the two commits
  vector<GitOid> oids{
    GitHexOid("49cf9b6d7ab307a4601e48e6a837bb329b3a14af").toOid(),
    GitHexOid("618aefc79b79482ff80f3bf6a8a6baf103868461").toOid()
  };
  
  GitOid zeroOid;
  
  constexpr uint32_t expectedHunkCount = 5;
  /* 16 is extremal element to calc last hunk linecount */
  vector<uint32_t> hunkLineBegins{1, 3, 4,  5, 13, 16};
  vector<uint32_t> hunkOidIds    {0, 1, 0,  1,  0};
  
  GitRepository repo(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));

  GitBlameOptions blameOpts;
  GitBlame blame(GitBlame::file(repo, "hello.cpp", blameOpts));

  auto hunkCount = blame.getHunkCount();
  ASSERT_EQ(expectedHunkCount, hunkCount);
  
  for (decltype(hunkCount) i = 0; i < hunkCount; ++i)
  {
    GitBlameHunk h = blame.getHunkByIndex(i);
    
    //EXPECT_EQ(hunkLineBegins[i], h.orig_start_line_number);
    EXPECT_EQ(oids[hunkOidIds[i]], h.orig_commit_id);

    EXPECT_EQ(hunkLineBegins[i], h.final_start_line_number);
    EXPECT_EQ(oids[hunkOidIds[i]], h.final_commit_id);

    uint32_t expectedHunkLength =
      hunkLineBegins[i + 1] - hunkLineBegins[i];
    EXPECT_EQ(expectedHunkLength, h.lines_in_hunk);
  }
  
}


TEST(GitParserBlameTests, BlameBuffer)
{
  //these are the two commits
  vector<GitOid> oids{
    GitOid(),
    GitHexOid("49cf9b6d7ab307a4601e48e6a837bb329b3a14af").toOid(),
    GitHexOid("befd3ff27f44c50c441589631b3935b06fdc3a09").toOid()
  };
  
  GitOid zeroOid;
  
  constexpr uint32_t expectedHunkCount = 3;
  /* 16 is extremal element to calc last hunk linecount */
  vector<uint32_t> hunkLineBegins{1, 4, 5,  8};
  vector<uint32_t> hunkOidIds    {0, 1, 2};
  
  GitRepository repo(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));

  GitBlameOptions blameOpts;
  GitBlame blame(GitBlame::file(repo, "Makefile", blameOpts));
  
  std::ifstream t(TOP_SRCDIR "parser/gitparser/test/test_src/simple/" "Makefile");
  std::stringstream buffer;
  buffer << t.rdbuf();

  blame = blame.blameBuffer(buffer.str());

  auto hunkCount = blame.getHunkCount();
  ASSERT_EQ(expectedHunkCount, hunkCount);
  
  for (decltype(hunkCount) i = 0; i < hunkCount; ++i)
  {
    GitBlameHunk h = blame.getHunkByIndex(i);
    
    //EXPECT_EQ(hunkLineBegins[i], h.orig_start_line_number);
    EXPECT_EQ(oids[hunkOidIds[i]], h.orig_commit_id);

    //EXPECT_EQ(hunkLineBegins[i], h.final_start_line_number);
    EXPECT_EQ(oids[hunkOidIds[i]], h.final_commit_id);

    uint32_t expectedHunkLength =
      hunkLineBegins[i + 1] - hunkLineBegins[i];
    EXPECT_EQ(expectedHunkLength, h.lines_in_hunk);
  }
  
}
