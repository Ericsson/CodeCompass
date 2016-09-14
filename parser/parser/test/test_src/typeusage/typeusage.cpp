/*
 * a.cpp
 *
 *  Created on: Jun 27, 2014
 *      Author: ezoltbo
 */

struct A
{
  void foo();
};

void A::foo()
{

}

struct B
{
  A m_a;
};

template<typename T>
struct ptr
{

};

A returnA()
{
  return {};
}

void takeA(A a)
{

}

void takePtrA(ptr<A> pa)
{

}

A globalA;

typedef A AAA;

int main()
{
  A localA;
}





