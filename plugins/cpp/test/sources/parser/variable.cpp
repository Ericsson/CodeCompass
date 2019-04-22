struct S{};

void f(int) {}
void g(int&) {}

void varContainingFunction()
{
  int variableDefinition;
  extern bool variableDeclaration;
  int (*variableFunctionPointer)(char, double);
  S variableUserType;

  f(variableDefinition);
  g(variableDefinition);
}
