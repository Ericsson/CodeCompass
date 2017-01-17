#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>
#include "servicehelper.h"

const char* dbConnectionString;

int main(int argc, char** argv)
{
  system(argv[1]);
  system(argv[2]);

  dbConnectionString = argv[3];

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
