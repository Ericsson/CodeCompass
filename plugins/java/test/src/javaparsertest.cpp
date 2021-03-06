#include <gtest/gtest.h>

class JavaParserTest : public ::testing::Test
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

TEST_F(JavaParserTest, simpleJavaParserTest)
{
  ASSERT_EQ(1,1);
}

TEST_F(JavaParserTest, simpleJavaParserTest2)
{
  ASSERT_TRUE(true);
}
