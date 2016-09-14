template<typename T>
void foo(T) { }

void bar(int) 
{ 
}

int main()
{
  foo(3);
  foo(3.14);
  bar(2);
}

