namespace MyNamespace1
{
namespace MyNamespace2
{
  void innerFunction () {}
}
}

void namespaceUserFunction()
{
  using namespace MyNamespace1;
  MyNamespace2::innerFunction();
}
