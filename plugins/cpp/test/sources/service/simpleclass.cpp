#include "simpleclass.h"

namespace cc
{
namespace test
{

SimpleClass::SimpleClass() : _locPrivX(-1), _locProtX(-1)
{
}

SimpleClass::~SimpleClass()
{
}

int SimpleClass::getPrivX() const
{
  return _locPrivX;
}

void SimpleClass::setPrivX(int x_)
{
  _locPrivX = x_;
}

} // test
} // cc
