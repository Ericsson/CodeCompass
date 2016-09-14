/*
 * a.cpp
 *
 *  Created on: Jun 27, 2014
 *      Author: ezoltbo
 */

void f()
{

}

void g()
{
  f();
}

class B
{
public:
  virtual void foo() = 0;

  virtual ~B() {};
};

class D : public B
{
public:
  void foo()
  {
    g();
  }
};


int main()
{
  B *pb = new D();

  pb->foo();

  delete pb;
}




