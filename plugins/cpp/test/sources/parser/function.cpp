void singleFunc()
{
  
}

void funcDecl();

void multiFunction();
void multiFunction();
void multiFunction() {}

int callee(char c, bool b)
{
  int i = b + c - 42;
  return i;
}

void caller()
{
  callee('x', true);
}
