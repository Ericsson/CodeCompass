void conditionals(int arg1, bool arg2) { // +1
  if (arg2) { } // +1
  else { }

  if ((arg1 > 5 && arg2) || (arg1 < 13 && !arg2)) { } // +4

  int local1 = arg2 ? arg1 / 2 : arg1 * 2; // + 1
  int local2 = local1 ?: arg1 + 5; // +1
} // 8

void loops1() { // +1
  for (int i = 0; i < 5; ++i) {} // +1

  int j = 0;
  while(j < 5) { ++j; } // +1

  do
  {
    ++j;
  } while (j < 10); // +1

  char str[] = "hello";

  for(char c : str) // +1
  {
    if (c == 'h') {} // +1
  }
} // 6

void loops2(int arg1, bool arg2) // +1
{
  while(arg2) // +1
  {
    if (arg1 < 5) // +1
    {
      arg1 *= 2;
      continue; // +1
    }
    else if (arg1 > 30) // +1
    {
      arg1 /= 2;
      continue; // +1
    }
    break;
  }
} // 6

int loops3(int arg1) // +1
{
  const int LIMIT = 42;
  int result = 0;
  for (int i = 0; i < arg1 * 2; ++i) // +1
  {
    ++result;
    for (int j = i; j < arg1 * 3; ++j) // +1
    {
      result -= j/5;
      if (result >= LIMIT) // +1
        goto endfor; // +1
    }
  }
  endfor:
  return result;
} // 5

void switchcase(int arg1) // +1
{
  switch(arg1)
  {
    case 1: // +1
      break;
    case 2: // +1
      break;
    case 3: // +1
      break;
    default: // +1
      switch(arg1 * 5) {
        case 85: // +1
          break;
        case 90: // +1
          break;
      }
      break;
  }
} // 7

class Err1 {};
class Err2 {};

void fragile(int arg1) // +1
{} // 1

void trycatch(int arg1) // +1
{
  try
  {
    fragile(arg1);
  }
  catch (Err1& err) // +1
  {

  }
  catch (Err2& err) // +1
  {

  }
} // 3

class MyClass
{
public:
  MyClass(unsigned int arg1) // +1
  {
    if (arg1 < LIMIT) // +1
      badPractice = new char[LIMIT];
    else
      badPractice = new char[arg1];
  } // 2

  ~MyClass() // +1
  {
    for (unsigned int i = 0; i < LIMIT; ++i) {} // +1
    delete[] badPractice;
  } // 2

  void method1(int arg1) // +1
  {
    for (unsigned int i = arg1; i < LIMIT; ++i) // +1
    {
      switch(arg1)
      {
        case -1: // +1
          goto endfor; // +1
        case 0: // +1
          break;
        case 1: // +1
          break;
        default: // +1
          continue; // +1
      }
      arg1 *= 2;
    }
    endfor:;
  } // 8
private:
  const unsigned int LIMIT = 42;
  char* badPractice;
};
