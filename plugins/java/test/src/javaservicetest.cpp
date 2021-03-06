#include <gtest/gtest.h>

class JavaServiceTest : public ::testing::Test
{
protected:  
  /**
   *  Prepare the objects for each test
   */
  virtual void SetUp() override
  {
  }
  
  /**
   * Release any resources you allocated in SetUp()
   */
  virtual void TearDown() override
  {  
  }
};

TEST_F(JavaServiceTest, simpleJavaServiceTest)
{
  EXPECT_EQ(7,7);
}
