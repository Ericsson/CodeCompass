
int global;


int g() { return 42; }

struct X {};

struct S
{
  int a;
  int* ap;
  const unsigned int* acp;

  unsigned int** app;
  unsigned int*** appp;


  int aa[42];
  int* aap[42];
  int (*apa)[42];
  int*& apr;
  S() : apr(ap)  {}
};

namespace nm
{

struct Bar {};

}

nm::Bar bar;

using namespace nm;

Bar barr;

int f(int arg) 
{
  const int x = 4;
  arg = x;
  S s;
  S* sp = &s;
  S& rs = s;
  sp->a = 7;
  ++rs.a;

  X xx; 
  X sa[12];
  X saa[3][3];
  sa[0] = xx;
  saa[0][0] = sa[2];
 
  return arg;
}

