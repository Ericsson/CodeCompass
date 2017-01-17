#ifndef CC_TEST_NESTEDCLASS_H
#define CC_TEST_NESTEDCLASS_H

namespace cc
{
namespace test
{

class NestedClass{
private:
  class InnerClass;

  struct InnerClass /*!< TODO: InfoTree: InnerClass */
  {
    int innerX;
    static const int staticInt;
    void f(int param_);
  };

public:
  static InnerClass createNestedClass();
};

struct List
{
  struct Node
  {
    int data;
    Node* next;
    Node* prev;
  };

  Node* head;
  Node* tail;
};

} // test
} // cc

#endif // CC_TEST_NESTEDCLASS_H
