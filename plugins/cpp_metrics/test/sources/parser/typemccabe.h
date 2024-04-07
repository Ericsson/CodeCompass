#ifndef TYPE_MCCABE__H
#define TYPE_MCCABE__H

class Empty {};

class ClassMethodsInside {
public:
  ClassMethodsInside(bool b) // +1
  {
    if (b) // +1
    {
      a = 1;
    }
    else
    {
      a = 0;
    }
  } // 2

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
private:
  int a;
  char *p;
}; // 16

class ClassMethodsOutside { // definitions are in typemccabe.cpp
  void conditionals(int arg1, bool arg2); // 8
  void loops1();                          // 6
  void loops2(int arg1, bool arg2);       // 6
  int loops3(int arg1);                   // 5
  void switchcase(int arg1);              // 7
  void fragile(int arg1);                 // 1
  void trycatch(int arg1);                // 3
  void method1(int arg1);                 // 8
}; // 44

class ClassMethodsInsideAndOutside {
  void foo() // +1
  {
    for (int i=0; i<3; ++i) // +1
    {
      for (int j=0; j<3; ++j) // +1
      {
        int x;
      }
    }
  } // 3

  bool bar(bool b1, bool b2, bool b3) // +1
  {
    return b1 && b2 && b3; // +2
  } // 3

  int baz(); // 2 (defined in typemccabe.cpp)
}; // 8

#endif // TYPE_MCCABE__H

