#include "inheritance.h"

namespace cc
{
namespace test
{

const int DerivedClass::Z = 10; /*!< TODO: InfoTree: DerivedClass */

BaseClass1::~BaseClass1()
{

}

void BaseClass1::f()
{

}

void BaseClass1::g()
{

}

BaseClass2::~BaseClass2()
{

}

void BaseClass2::f()
{

}

void BaseClass2::h() const
{

}

void DerivedClass::f()
{
  _protBaseX = 10;
}

void DerivedClass::g()
{

}

void DerivedClass::h() const
{

}

} // test
} // cc
