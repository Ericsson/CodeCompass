#include <memory>
#include <vector>
#include <unordered_map>

namespace CC_CPP_EFFERENT_COUPLING_METRICS_TEST
{
// Member types
class A {};

class A1 {
  int a;
  bool b;

  void foo(char* c);
};

class A2 {
  A a;
};

class A3 {
  A* a;
};

class A4 {
  A& a;
};

class A5 {
  std::vector<A> a;
};

class A6 {
  std::unique_ptr<A> a;
};

class A7 {
  int i;
  A a;
};

// ==========

// Function parameters
class B {
  void foo(A a);
};

class B2 {
  void foo(A* a);
};

class B3 {
  void foo(A& a);
};

class B4 {
  void foo(std::vector<A>& a);
};

class B5 {
  void foo(std::unique_ptr<A>& a);
};

class B6 {
  void foo(int, bool, A* a);
};

class B7 {
  void foo(int, bool);
  void foo(int, bool, A* a);
};

// ==========

// Function local variables
class C {
  void foo() { A a; }
};

class C2 {
  void foo() { A* a; }
};

class C3 {
  void foo() { std::vector<A> a; }
};

class C4 {
  void foo() { auto a = A(); }
};

class C5 {
  void foo();
};

void C5::foo() { A a; }

class C6 {
  void foo();
};

void C6::foo() { std::unique_ptr<A> a; }
// ==========

// Function return types
class D {
  A foo() { return A(); };
};

class D2 {
  A* foo() { return nullptr; };
};

class D3 {
  std::vector<A> foo();
};

// Inheritance
class E : public A {};
// ==========

// Multiple usage of the same type
class F : public A {
  A* a;

  void foo()
  {
    A a;
  }

  void bar(std::vector<A> a_vec);
};

class F2 {
  std::unique_ptr<A> a;

  A foo();
};

A F2::foo() { return A(); }

// ==========

// Depends on multiple types
class G {
  A a;
  B b;
};

class G2 : public A, public B {};

class G3 {
  A foo() { return A(); };

  std::vector<B> b;
};

class G4 : public A {
  A a;

  void foo(A& a);

  void bar()
  {
    B b;
  }
};

class G5 {
  void foo();
};

void G5::foo()
{
  auto a = A();
  B b;
}

class G6 {
  A a;

  void foo(std::vector<A>&, B b);
};

class G7 {
  A foo() { return A(); }
  B bar() { return B(); }
};

class G8 {
  std::unordered_map<A, B> map;
};
// ==========
}
