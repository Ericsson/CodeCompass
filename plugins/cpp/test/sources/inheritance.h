#ifndef CC_TEST_INHERITANCE_H
#define CC_TEST_INHERITANCE_H

namespace cc
{
namespace test
{

class BaseClass1{
public:
  virtual ~BaseClass1();
  virtual void f() = 0;
  virtual void g();

  int _pubBaseX;

protected:
  int _protBaseX;
};

class BaseClass2{
public:
  virtual ~BaseClass2();
  virtual void f() = 0;
  virtual void h() const;

protected:
  int _protBaseY;
};

class DerivedClass : protected BaseClass1, protected BaseClass2
{
public:
  static const int Z;

  void f() override;

  void g() override;

  void h() const override;
};

} // test
} // cc

#endif // CC_TEST_INHERITANCE_H
