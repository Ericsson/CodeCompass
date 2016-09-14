/*
 * gitparserblobtest.cpp
 *
 *  Created on: Mar 2, 2014
 *      Author: cseri
 */

#include <gtest/gtest.h>

#include <util/streamlog.h>

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"
#include "gitparser/gitoid.h"
#include "gitparser/githexoid.h"
#include "gitparser/gitblob.h"

using namespace cc::parser;


TEST(GitParserBlobTests, LookUpFail)
{
  GitOid oid;
  
  GitRepository r_empty(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/empty/"));
  ASSERT_THROW(
    {
      GitBlob b(GitBlob::lookUp(r_empty, oid));
    },
    GitException
  );

  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  ASSERT_THROW(
    {
      GitBlob b(GitBlob::lookUp(r_simple, oid));
    },
    GitException
  );
}

TEST(GitParserBlobTests, LookUp)
{
  GitOid oid = GitHexOid("b38652672d5159c46bc8fb08cfb976fceeef6786").toOid();
  
  GitRepository r_simple(GitRepository::open(TOP_SRCDIR "parser/gitparser/test/test_src/simple/"));
  GitBlob b = GitBlob::lookUp(r_simple, oid);
  
  ASSERT_EQ(92U, b.getDataSize());

  std::string data = b.getDataAsString();
  ASSERT_EQ(92U, data.size());
  ASSERT_EQ(56U, data.find("World"));
}

