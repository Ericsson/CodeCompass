#include <memory>

struct X
{
public:
	X(){}
	X(int x, int y, int z){}
  X(int x, int y){}
  void f(int& x, int& y){}
  int p;
};

struct Y
{
  static inline void f(const X* const param)
  {    
  } 
};

int main(int argc, char* argv[])
{
  return 0;
}

void foo3(int& x3){}
void foo2(int& x2)
{
  foo3(x2);
  int *xx;
  xx = &x2; 
  int* xx2 = &x2; 
  int ** xx3 = &xx2;
}

void foo1(int& x1)
{
  foo2(x1);
}

void test1() 
{
  int param; // param, x1, x2, x3, xx2, xx, xx3, param2
  foo1(param);

  int param2; 
  foo1(param2);
}

void test2()
{
  int cx = 0; // c1
  int cy = 0; // c2
  auto f = [&](int &c1, int& c2){ 
    return &cx; 
  };
  f(cx,cy);
}

void test3()
{
  int x, y; // x, y, p, q
  int *p = &x; 
	int *&q = p;
	p = &y;
}

void test4()
{
  int param, // x
      param1, // x1
      param2; // x2
  auto b = [](int& x1, int & x2){return true;};
  auto f = [](int& x){return x;};
  if(f(param) == 10 || b(param1, param2)){}
}

void test5()
{
  X *x = new X(); // x, construct call X, xx
  auto xx = &x; 
}

void test6()
{
  int x,y;
  auto f = [](X* x){}; // construct call X
  f(new X(x,y,2));
}

void test7()
{
  X *x; // param1
  auto f1 = [&](X** param1){ }; 
  f1(&x); 
}

void test8()
{
  X x; // param
  Y::f(&x);
}

void test9()
{
	X *x = new X();
	int &ref = x->p; // p
}

void test10()
{
  int i = 1; //x,y, pt2, p4, pr, p1, pp2, pt, p2, p3, pp, pxx, pw, px1
  int &x = i;
  int &y = x;
  int *p1 = &i;
  int **p2 = &p1;
  int **p3 = p2;
  int *p4 = *p2;

  int* &pr = p1;
  int** pp = &*p2;
  int* pp2 = &**p2;

  int **pw = &pp2;
  int ***px1 = &p2;
  px1 = &pw;

  int **pxx = &**px1;

  pxx = nullptr;
  pw = 0;

  int *pt2;
  int **pt;
  pt = &pt2;
  *pt = &i;
}

void test11()
{
  std::shared_ptr<X> sp = std::make_shared<X>(); // template call, sp2, spp
	std::shared_ptr<X> sp2 = sp;
  std::shared_ptr<X> *spp = &sp;
}

void test12()
{
  auto foo = [&](const std::shared_ptr<X>& param){};

  std::shared_ptr<X> p1(new X); //construct call, p2, p3, p4, weakP1, param, sp, template call, anonymus, anonymus 
  std::shared_ptr<X> p2 = p1;
  std::shared_ptr<X> p3 = p2;
  std::shared_ptr<X> p4 = p3;

  p2 = nullptr;

  auto sp = std::make_shared<X>(1,2);
  sp = p1;
  foo(sp);

  std::weak_ptr<X> weakP1 = p1; 

  std::unique_ptr<X> up1(new X); // construct call, moveUP3, moveUP2, __t
  std::unique_ptr<X> moveUP2(std::move(up1));
  std::unique_ptr<X> moveUP3(std::move(moveUP2));
}