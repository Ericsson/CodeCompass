#ifndef CC_TEST_SIMPLECLASS_H
#define CC_TEST_SIMPLECLASS_H

namespace cc
{
namespace test
{

class SimpleClass
{
public:

  SimpleClass();

  ~SimpleClass();

  int getPrivX() const;

  void setPrivX(int x_);

protected:
  int _locProtX;

private:
  int _locPrivX;
};

} // test
} // cc

#endif // CC_TEST_SIMPLECLASS_H
