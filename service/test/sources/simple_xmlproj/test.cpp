#ifdef FOO
void foo()
{
}
#endif

#ifdef BAR
void bar()
{
}
#endif

int main()
{
#ifdef FOO
  foo();
#endif

#ifdef BAR
  bar();
#endif
  return 0;
}

