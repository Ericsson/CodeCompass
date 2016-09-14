int f(int a, int b)
{ 
  int c;

  c = b;
  c += a;
  c -= b;

  ++b;
  b--;

  return b;
}

struct A
{
  void foo() {}

  void bar() const {}

  int i;
};

void funcRefs(A& a, const A& b)
{

}

void funcPtrs(A* a, const A* b)
{

}

int  main()
{
  A aaa;

  aaa.foo();
  aaa.bar();

  funcRefs(aaa, aaa);
  funcPtrs(&aaa, &aaa);

  aaa.i = 42 + aaa.i;

  return aaa.i;
}

