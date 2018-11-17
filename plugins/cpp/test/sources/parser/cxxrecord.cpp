class MyClass;
class MyClass
{
public:
  virtual void virtualFunction() {}
  void nonVirtualFunction () {}
  int memberVariable;
};

MyClass globalVariable;

MyClass function(
  const MyClass& param)
{
  MyClass localVariable;
  MyClass localVariable2(localVariable);
  MyClass localVariable3 = localVariable;
  try
  {
    throw MyClass();
  }
  catch (const MyClass&)
  {
  }
  return localVariable;
}

typedef MyClass MyClass2;

struct Derived : public MyClass
{
  MyClass fieldVariable;
  MyClass (*fieldFunction)(const MyClass&);
  Derived() :
    fieldVariable(MyClass()),
    fieldFunction(function)
  {
    MyClass* ptr = new MyClass;
    ptr->virtualFunction();
    ptr->nonVirtualFunction();
    delete ptr;
    ptr = new MyClass[10];
    delete[] ptr;
    ptr->memberVariable;
    ptr->memberVariable = 42;
  }
  void memberFunction();
};

void Derived::memberFunction() {}
