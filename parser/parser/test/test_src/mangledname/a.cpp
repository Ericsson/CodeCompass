int global = 42;

int f(int a, char b)
{
  return a + b;
}

int main()
{
  int a = f(1,'a');
  {
    int b = a;
    int a = f(2, 'x');
    b += a;
  }

  return a + global;  
}
