namespace
{

class LocalClass
{
public:
  LocalClass() :
    _privX(42)
  {
  }

  ~LocalClass()
  {
  }

public:
  int getPrivX()
  {
    return _privX;
  }

  int getPrivX() const
  {
    return _privX;
  }

  void setPrivX(int x_)
  {
    _privX = x_;
  }

private:
  int _privX;
};

}

int main(int argc_, char* argv_[])
{
  int locaVar1 = 0;

  LocalClass locaVar2;
  locaVar2.setPrivX(0);
  locaVar2.getPrivX();

  return 0;
}

//----------------------------------------------------------------
// here starts the implicit code testing source

struct MyStruct
{
  MyStruct() { }
  MyStruct(const MyStruct&) { }
  MyStruct(MyStruct&&) { }
  ~MyStruct() { }
  MyStruct& operator=(const MyStruct&) { }
};

struct ImplicitTest
{
  MyStruct s1, s2;
};

void fun()
{
  ImplicitTest b1;

  {
    ImplicitTest b2 = b1;
    b1 = b2;
  }
}
