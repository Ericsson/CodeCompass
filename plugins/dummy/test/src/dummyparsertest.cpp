#include <gtest/gtest.h>

class DummyParserTest : public ::testing::Test
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

TEST_F(DummyParserTest, simpleDummyParserTest)
{
  ASSERT_EQ(1,1);
}

TEST_F(DummyParserTest, simpleDummyParserTest2)
{
  ASSERT_TRUE(true);
}
