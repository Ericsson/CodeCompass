#include <gtest/gtest.h>

const char* dbConnectionString;

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    GTEST_LOG_(FATAL) << "Test arguments missing.";
    return 1;
  }

  GTEST_LOG_(INFO) << "Testing Python started...";
  system(argv[1]);

  GTEST_LOG_(INFO) << "Using database for tests: " << dbConnectionString;
  dbConnectionString = argv[2];
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
