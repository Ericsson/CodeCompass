namespace Simple
{
  /// Sum function @ref Simple::SimpleClass::g()
  int f(int a, int b)
  {
    return a+b;
  }

  /// Class for documentation comment parsing test
  class SimpleClass
  {
  public:
		/// @ref g()
    SimpleClass() {}

    /** @param a first
        @param b second
    */
    int g(int a, int b)
    {
      return a+b;
    }
  };
}

int main()
{
  int a = f(2,4);
  Simple::SimpleClass s;
  s.g(2,4);

  return 0;
}
