#include <memory>
#include <vector>

// Member types
class A {};
class A_D {
  A a;
};

class A2 {};
class A2_D {
  A2* a2;
};

class A3 {};
class A3_D {
  A3& a3;
};

class A4 {};
class A4_D {
  std::vector<A4> a4_vec;
};

class A5 {};
class A5_D {
  std::unique_ptr<A5> a5_ptr;
};
// ==========

// Function parameters
class B {};
class B_D {
  void foo(B b);
};

class B2 {};
class B2_D {
  void foo(B2* b);
};

class B3 {};
class B3_D {
  void foo(B3& b);
};

class B4 {};
class B4_D {
  void foo(std::vector<B4>& b4_vec);
};

class B5 {};
class B5_D {
  void foo(std::unique_ptr<B5>& b5_ptr);
};

class B6 {};
class B6_D {
  void foo(int, bool, B6* b);
};

class B7 {};
class B7_D {
  void foo(int, bool, B7* b);
};

class B8 {};
class B8_D {
  void foo(int, bool);
  void foo(int, bool, B8* b);
};
// ==========

// Function local variables
class C {};
class C_D {
  void foo()
  {
    C c;
  }
};

class C2 {};
class C2_D {
  void foo()
  {
    auto c2 = C2();
  }
};

class C3 {};
class C3_D {
  void foo()
  {
    std::vector<C3> c3_vec;
  }
};
// ==========

// Out-of-class member functions
class D {};
class D_D {
  void foo();
};

void D_D::foo()
{
  D d;
}

class D2 {};
class D2_D {
  void foo();
};

void D2_D::foo()
{
  std::unique_ptr<D2> d2_ptr;
}
// ==========

// Multiple usage of the same type
class E {};
class E_D {
  E* e;

  void foo()
  {
    E e;
  }

  void bar(std::vector<E> e_vec);
};
// ==========

// Inheritance
class F {};
class F_D : public F {};
// ==========

// Multi inheritance
class G {};
class G_C {};
class G_D : public G_C, public G {};
// ==========

// Multiple dependant types
class H {};
class H_D1 {
  H h;
};

class H_D2 {
  void foo()
  {
    std::vector<H> h_vec;
  }
};

// ----------

class H2 {};
class H2_D1 {
  H2 h2;
  void bar(std::unique_ptr<H2> h2_ptr);
};

class H2_D2 {
  H2* h2_ptr;

  void foo()
  {
    std::vector<H2> h2_vec;
  }
};
// ==========
