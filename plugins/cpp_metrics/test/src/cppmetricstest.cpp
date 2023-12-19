#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <util/dbutil.h>

const char* dbConnectionString;

int main(int argc, char** argv)
{
  dbConnectionString = argv[3];
  if (strcmp(dbConnectionString, "") == 0)
  {
    GTEST_LOG_(FATAL) << "No test database connection given.";
    return 1;
  }

  GTEST_LOG_(INFO) << "Testing C++ metrics plugin started...";
  GTEST_LOG_(INFO) << "Executing build command: " << argv[1];
  system(argv[1]);

  GTEST_LOG_(INFO) << "Executing parser command: " << argv[2];
  system(argv[2]);

  GTEST_LOG_(INFO) << "Using database for tests: " << dbConnectionString;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
