#include <servicetest/servicetest.h>

#include "../src/searchservice.h"

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <thrift/Thrift.h>

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iostream>

using namespace cc::service::core;
using namespace cc::service::search;

namespace cc
{
namespace service
{
namespace core
{

bool Position::operator < (const Position& other_) const
{
  return line < other_.line || (line == other_.line && column < other_.column);
}

bool Range::operator < (const Range& other_) const
{
  return startpos < other_.startpos ||
    (startpos == other_.startpos && endpos < other_.endpos);
}

} // namespace core
} // namespace service
} // namespace cc

namespace
{

using StringSet = std::unordered_set<std::string>;
using ExpectedMatches = std::map<std::string, std::set<Range>>;

bool resultSetEq(const StringSet& names_, const SearchResult& results_)
{
  EXPECT_EQ(names_.size(), results_.results.size());

  std::size_t cnt = 0;
  for (const auto& ent : results_.results)
  {
    std::cout << "Found " << ent.finfo.name << std::endl;

    EXPECT_EQ(1, names_.count(ent.finfo.name));
    cnt += names_.count(ent.finfo.name);
  }

  EXPECT_EQ(names_.size(), cnt);

  return
    names_.size() == results_.results.size() &&
    names_.size() == cnt;
}

Range makeRange(int sl_, int sc_, int el_, int ec_)
{
  Range range;
  range.startpos.line = sl_;
  range.startpos.column = sc_;
  range.endpos.line = el_;
  range.endpos.column = ec_;

  return range;
}

bool checkMatches(ExpectedMatches matches_, const SearchResult& res_)
{
  EXPECT_EQ(matches_.size(), res_.results.size());

  bool result = true;
  for (const auto& ent : res_.results)
  {
    std::cout << "Checking matches in " << ent.finfo.name << std::endl;

    auto rngSetIter = matches_.find(ent.finfo.name);
    EXPECT_FALSE(rngSetIter == matches_.end());
    if (rngSetIter == matches_.end())
    {
      continue;
    }

    const std::set<Range>& rngSet = rngSetIter->second;
    EXPECT_EQ(rngSet.size(), ent.matchingLines.size());

    for (const auto& match : ent.matchingLines)
    {
      std::cout << "Found range:"
        << ' ' << match.range.range.startpos.line
        << ' ' << match.range.range.startpos.column
        << ' ' << match.range.range.endpos.line
        << ' ' << match.range.range.endpos.column
        << std::endl;

      EXPECT_EQ(ent.finfo.file, match.range.file);
      EXPECT_EQ(1, rngSet.count(match.range.range));

      result = result && rngSet.count(match.range.range) == 1;
    }
  }

  return result;
}

} // anonymous namespace

class SearchServiceTest : public cc::servicetest::ServiceTestBase
{
};

TEST_F(SearchServiceTest, SearchTypesTest)
{
  SearchServiceHandler srv(EmptyDatabase(this));

  std::vector<SearchType> types;
  srv.getSearchTypes(types);

  EXPECT_EQ(4, types.size());
}

TEST_F(SearchServiceTest, FileFilterTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));

  SearchParams params;
  params.__isset.filter = true;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "*";

  StringSet resSrcFiles = { "tinyxml2.cpp", "xmltest.cpp" };
  StringSet resHFiles = { "tinyxml2.h", "xmltest.h" };
  StringSet resAllFiles = {  "tinyxml2.cpp", "xmltest.cpp", "tinyxml2.h",
    "xmltest.h" };

  // tinyxml2.cpp
  {
    SearchResult res;
    params.filter.fileFilter = "tinyxml2\\.cpp";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq({ "tinyxml2.cpp" }, res));
  }

  // tinyxml2.cpp (case-insensitive)
  {
    SearchResult res;
    params.filter.fileFilter = "TINYxml2\\.cpp";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq({ "tinyxml2.cpp" }, res));
  }

  // tinyxml2.cpp
  {
    SearchResult res;
    params.filter.fileFilter = "tinyxml2.cpp";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq({ "tinyxml2.cpp" }, res));
  }

  // Ends with cpp
  {
    SearchResult res;
    params.filter.fileFilter = ".*cpp";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resSrcFiles, res));
  }

  // Ends with .cpp
  {
    SearchResult res;
    params.filter.fileFilter = ".*\\.cpp";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resSrcFiles, res));
  }

  // Ends with h
  {
    SearchResult res;
    params.filter.fileFilter = ".*h";
    srv.search(res, params);
    EXPECT_TRUE(resultSetEq(resHFiles, res));
  }

  // All
  {
    SearchResult res;
    params.filter.fileFilter = ".*";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // All
  {
    SearchResult res;
    params.filter.fileFilter = "";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // All
  {
    SearchResult res;
    params.filter.fileFilter = ".*xml.*";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // None
  {
    SearchResult res;
    params.filter.fileFilter = "No match";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(StringSet{}, res));
  }
}

TEST_F(SearchServiceTest, DirFilterTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));

  SearchParams params;
  params.__isset.filter = true;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "*";

  StringSet resAllFiles = {  "tinyxml2.cpp", "xmltest.cpp", "tinyxml2.h",
    "xmltest.h" };

  // All
  {
    SearchResult res;
    params.filter.dirFilter = "";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // All
  {
    SearchResult res;
    params.filter.dirFilter = ".*/sources/tiny.*";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // All (case-insensitive)
  {
    SearchResult res;
    params.filter.dirFilter = ".*/SOUrcES/tiny.*";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(resAllFiles, res));
  }

  // None
  {
    SearchResult res;
    params.filter.dirFilter = "No match";
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(StringSet{}, res));
  }
}

TEST_F(SearchServiceTest, FilterTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));

  SearchParams params;
  params.__isset.filter = true;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "*";
  params.filter.dirFilter = ".*/sources/tiny.*";
  params.filter.fileFilter = ".*\\.cpp";

  SearchResult res;
  srv.search(res, params);

  EXPECT_TRUE(resultSetEq({"tinyxml2.cpp", "xmltest.cpp" }, res));
}

TEST_F(SearchServiceTest, TextSearchRangeTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));

  SearchParams params; 
  params.options = SearchOptions::SearchInSource;
  params.query.query = "*";
  params.range.maxSize = 1;
  params.__isset.range = true;

  {
    params.range.start = 0;
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_EQ(0, res.firstFileIndex);
    ASSERT_EQ(1, res.results.size());
  }
  
  {
    params.range.start = 1;
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_EQ(1, res.firstFileIndex);
    ASSERT_EQ(1, res.results.size());
  }

  {
    params.range.start = 2;
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_EQ(2, res.firstFileIndex);
    ASSERT_EQ(1, res.results.size());
  }
}

TEST_F(SearchServiceTest, TextSearchBadTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "\"LocalClass";
    
  SearchResult res;
  ASSERT_THROW(srv.search(res, params), apache::thrift::TException);
}

TEST_F(SearchServiceTest, TextSearchSimpleTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));

  SearchParams params;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "LocalClass";

  SearchResult res;
  srv.search(res, params);

  EXPECT_TRUE(checkMatches({
  {
    "simple.cpp", {
      makeRange(4, 7, 4, 17),
      makeRange(7, 3, 7, 13),
      makeRange(12, 4, 12, 14),
      makeRange(42, 3, 42, 13)
    }
  }
  }, res));
}

TEST_F(SearchServiceTest, TextSearchOrTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "locaVar1 OR argc_ OR NOOOOOOOO";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(38, 14, 38, 19),
        makeRange(40, 7, 40, 15)
      }
    }
    }, res));
  }

  {
    params.query.query = "locaVar1 argc_ NOOOOOOOO";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(38, 14, 38, 19),
        makeRange(40, 7, 40, 15)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, TextSearchAndTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "locaVar1 AND argc_ AND NOOOOOOOO";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq(StringSet{}, res));
  }

  {
    params.query.query = "locaVar1 AND argc_";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(38, 14, 38, 19),
        makeRange(40, 7, 40, 15)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, TextSearchLoveHateTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "+example_1 -purely";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "xmltest.cpp", {
        makeRange(81, 5, 81, 14),
        makeRange(179, 27, 179, 36)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, TextSearchStarTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "locavar*";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(40, 7, 40, 15),
        makeRange(42, 14, 42, 22),
        makeRange(43, 3, 43, 11),
        makeRange(44, 3, 44, 11)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, TextSearchRegexTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "/l.c.*ar[1-2]/";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(40, 7, 40, 15),
        makeRange(42, 14, 42, 22),
        makeRange(43, 3, 43, 11),
        makeRange(44, 3, 44, 11)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, TextSearchBadRegexTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "/l.c.*ar[1-2]";
    
  SearchResult res;
  ASSERT_THROW(srv.search(res, params), apache::thrift::TException);
}

TEST_F(SearchServiceTest, TextSearchPhraseTest)
{
  SearchServiceHandler srv(SimpleDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchInSource;

  {
    params.query.query = "locaVar2.getPrivX";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(17, 7, 17, 15),
        makeRange(22, 7, 22, 15),
        makeRange(42, 14, 42, 22),
        makeRange(43, 3, 43, 11),
        makeRange(44, 3, 44, 20)
      }
    }
    }, res));
  }

  {
    params.query.query = "\"locaVar2.getPrivX\"";
    
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp", {
        makeRange(44, 3, 44, 20)
      }
    }
    }, res));
  }
}

TEST_F(SearchServiceTest, MimeFilterTest)
{
  SearchServiceHandler srv(SearchMixDatabase(this));

  SearchParams params;
  params.options = SearchOptions::SearchInSource;
  params.query.query = "toCelcius ";
  params.filter.dirFilter = ".*searchmix/mixedtype/dir1";
  params.__isset.filter = true;

  {
    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(resultSetEq({
      "simple.cpp",
      "simple.java",
      "simple.js",
      "simple.pl",
      "simple.py",
      "simple.sh",
      "simple.sql"
    }, res));
  }

  // C/C++
  {
    SearchParams qparams(params);
    qparams.query.query +=
      "AND (mime:(\"text/x-c++\") OR mime:(\"text/x-c\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.cpp",}, res));
  }

  // Java
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-java\") OR " \
      "mime:(\"text/x-java-source\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.java",}, res));
  }

  // JavaScript
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-javascript\") OR " \
      "mime:(\"application/javascript\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.js",}, res));
  }

  // Shell
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-shellscript\") OR " \
      "mime:(\"application/shellscript\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.sh",}, res));
  }

  // Python
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-python\"))";
    
    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.py",}, res));
  }

  // Perl
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-perl\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.pl",}, res));
  }

  // Custom: SQL
  {
    SearchParams qparams(params);
    qparams.query.query += "AND (mime:(\"text/x-sql\") OR " \
      "mime:(\"application/x-sql\"))";

    SearchResult res;
    srv.search(res, qparams);
    EXPECT_TRUE(resultSetEq({"simple.sql",}, res));
  }
}

TEST_F(SearchServiceTest, DefSearchMixedTest)
{
  SearchServiceHandler srv(SearchMixDatabase(this));

  SearchParams params;
  params.options = SearchOptions::SearchInDefs;
  params.filter.dirFilter = ".*searchmix/mixedtype/dir1";
  params.__isset.filter = true;

  {
    params.query.query = "toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
      { "simple.cpp", { makeRange(3, 8, 3, 17) } },
      { "simple.java", { makeRange(2, 24, 2, 33) } },
      { "simple.js", { makeRange(2, 10, 2, 19) } },
      { "simple.pl", { makeRange(1, 5, 1, 14) } },
      { "simple.py", { makeRange(1, 5, 1, 14) } },
      { "simple.sh", { makeRange(3, 10, 3, 19) } },
      { "simple.sql", { makeRange(1, 28, 1, 37) } }
    }, res));
  }

  {
    params.query.query = "defs:toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
      { "simple.cpp", { makeRange(3, 8, 3, 17) } },
      { "simple.java", { makeRange(2, 24, 2, 33) } },
      { "simple.js", { makeRange(2, 10, 2, 19) } },
      { "simple.pl", { makeRange(1, 5, 1, 14) } },
      { "simple.py", { makeRange(1, 5, 1, 14) } },
      { "simple.sh", { makeRange(3, 10, 3, 19) } },
      { "simple.sql", { makeRange(1, 28, 1, 37) } }
    }, res));
  }
}

TEST_F(SearchServiceTest, DefSearchCppQualifiedTest)
{
  SearchServiceHandler srv(SearchMixDatabase(this));

  SearchParams params;
  params.options = SearchOptions::SearchInDefs;
  params.filter.dirFilter = ".*searchmix/mixedtype/dir2";
  params.__isset.filter = true;

  {
    params.query.query = "toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp",
      {
        makeRange(6, 8, 6, 17),
        makeRange(21, 8, 21, 17)
      }
    }
    }, res));
  }

  {
    params.query.query = "xx\\:\\:toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp",
      {
        makeRange(21, 1, 21, 36)
      }
    }
    }, res));
  }

  {
    params.query.query = "*\\:\\:toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
    {
      "simple.cpp",
      {
        makeRange(6, 1, 6, 36),
        makeRange(21, 1, 21, 36)
      }
    }
    }, res));
  }

  // FIXME: searching in anonymous namespace is not supported
}

TEST_F(SearchServiceTest, DefSearchKindFilterTest)
{
  SearchServiceHandler srv(SearchMixDatabase(this));

  SearchParams params;
  params.options = SearchOptions::SearchInDefs;
  params.filter.dirFilter = ".*searchmix/mixedtype/dir1";
  params.__isset.filter = true;

  {
    params.query.query = "toCelcius AND func:toCelcius";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
      { "simple.cpp", { makeRange(3, 1, 3, 36) } },
      { "simple.java", { makeRange(2, 24, 2, 33) } },
      { "simple.js", { makeRange(2, 10, 2, 19) } },
      { "simple.pl", { makeRange(1, 5, 1, 14) } },
      { "simple.py", { makeRange(1, 5, 1, 14) } },
      { "simple.sh", { makeRange(3, 10, 3, 19) } },
      { "simple.sql", { makeRange(1, 28, 1, 37) } }
    }, res));
  }

  {
    params.query.query = "fahrenheit AND var:fahrenheit";

    SearchResult res;
    srv.search(res, params);

    EXPECT_TRUE(checkMatches({
      { "simple.cpp", { makeRange(3, 18, 3, 35) } }
    }, res));
  }
}

TEST_F(SearchServiceTest, FileNameSearchTest)
{
  SearchServiceHandler srv(TinyXmlDatabase(this));
  
  SearchParams params;
  params.options = SearchOptions::SearchForFileName;

  {
    params.query.query = "tinyxml2\\.cpp";
    
    FileSearchResult res;
    srv.searchFile(res, params);
    
    EXPECT_EQ(1, res.totalFiles);
    EXPECT_EQ(1, res.results.size());
    ASSERT_TRUE(res.results.size() > 0);
    EXPECT_STREQ("tinyxml2.cpp", res.results[0].name.c_str());
  }

  {
    params.query.query = ".*x..[2-7]\\.cpp";
    
    FileSearchResult res;
    srv.searchFile(res, params);
    
    EXPECT_EQ(1, res.totalFiles);
    EXPECT_EQ(1, res.results.size());
    ASSERT_TRUE(res.results.size() > 0);
    EXPECT_STREQ("tinyxml2.cpp", res.results[0].name.c_str());
  }

  // tinyxml2.cpp (case-insensitive)
  {
    params.query.query = "TINYxml2\\.cpp";
    
    FileSearchResult res;
    srv.searchFile(res, params);
    
    EXPECT_EQ(1, res.totalFiles);
    EXPECT_EQ(1, res.results.size());
    ASSERT_TRUE(res.results.size() > 0);
    EXPECT_STREQ("tinyxml2.cpp", res.results[0].name.c_str());
  }
}

TEST_F(SearchServiceTest, LogSearchTest)
{

}

int main(int argc, char *argv[])
{
  cc::util::StreamLog::setStrategy(std::shared_ptr<cc::util::LogStrategy>(
    new cc::util::StandardErrorLogStrategy()));
  cc::util::StreamLog::setLogLevel(cc::util::DEBUG);

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

