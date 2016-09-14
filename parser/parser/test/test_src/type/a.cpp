struct S
{
  long int a;
  long long int a2;
  short int a3;
  unsigned a4;
  unsigned long int a5;
  signed int a6;
  signed char a7;
  unsigned char a8;
  char a9;
  float a10;
  double a11;
  long double a12;
  int b;
  S() { }
  void foo() { a = 5; }
};

int main()
{
  S s;
  return 0;
}

