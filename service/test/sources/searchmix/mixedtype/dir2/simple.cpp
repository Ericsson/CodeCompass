#include <iostream>

namespace
{

double toCelcius(double fahrenheit)
{
  return (5/9) * (fahrenheit-32);
}

void sayHello()
{
  std::cout << "Hello!" << std::endl;
}

} // <anonymous namespace>

namespace xx
{

double toCelcius(double fahrenheit)
{
  return (5/9) * (fahrenheit-32);
}

void sayHello()
{
  std::cout << "Hello!" << std::endl;
}

} // namespace xx
