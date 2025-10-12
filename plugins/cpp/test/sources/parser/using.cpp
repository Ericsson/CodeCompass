namespace Nested
{
namespace MyNamespace
{
class C
{
};

void g(C) { }

void g(double) { }

constexpr C VAR1{};

template<class T>
constexpr T VAR2 = T{};
}
}

using namespace Nested;
void g() {}
using MyNamespace::g;

void using_fun()
{
  using MyNamespace::C;
  using MyNamespace::VAR1;
  using MyNamespace::VAR2;

  g();
  g(VAR1);
  g(VAR2<double>);
}