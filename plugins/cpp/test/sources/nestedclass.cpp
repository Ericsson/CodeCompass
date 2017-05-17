#include "nestedclass.h"

namespace cc
{
namespace test
{

const int NestedClass::InnerClass::staticInt = 1;

void NestedClass::InnerClass::f(int param_) /*!< TODO: InfoTree: NestedClass and InnerClass */
{
}

NestedClass::InnerClass NestedClass::createNestedClass() /*!< TODO: InfoTree: first NestedClass */
{
  return InnerClass(); /*!< TODO: InfoTree: InnerClass */
}

} // test
} // cc
