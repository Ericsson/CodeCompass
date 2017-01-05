/*
 * a.cpp
 *
 *  Created on: Jun 27, 2014
 *      Author: ezoltbo
 */

typedef int(*Foo)();

int function()
{
  return 42;
}

Foo f = function;

int boo(Foo f)
{
  return f();
}


struct A
{
  A(Foo f) : mFoo(f)
  {
    mFoo();
  }

  void bar()
  {
    mFoo();
  }

  Foo mFoo;
};

int woo() {return 42;}
int war() {return 36;}

int (*const methodsA []) () = {
  woo, war
};

int call(int (*const *methodsP)() )
{
  return methodsP[0]() + methodsP[1]();
}

int main()
{
  boo(function);

  f();

  Foo ff(f);

  ff();

  A a(ff);

  a.bar();

  call(methodsA);
}



