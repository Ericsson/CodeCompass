#ifndef SERVICE_CPPSERVICEHELPER_SLICER_PDG_MULTISTACK_H
#define SERVICE_CPPSERVICEHELPER_SLICER_PDG_MULTISTACK_H

#include <stack>
#include <vector>

/* 
 * TODO: handle functoin invocation in expressions
 * TODO: handle break, continue, return --> switch
 */
namespace cc
{ 
namespace service
{  
namespace language
{

/**
 * This class is used by PDG_builder to compute back control edges.
 * This is a extended stack implementation that allows us to store arbitrary elements in a slot of the stack.
 */
template<typename T>
class multistack
{
public:
  void push(const T& t)
  {
    ms.push(std::vector<T>()); 
    ms.top().push_back(t);    
  }
  
  void replace_top(const T& t)
  {
    std::vector<T> tmp{t};
    ms.top() = tmp;
  }
  
  void join_top2()
  {
    std::vector<T> tmp = ms.top();
    ms.pop();
    for(const T& t : tmp)
      ms.top().push_back(t);
  }
  
  std::vector<T>& top()
  {
    return ms.top();
  }
  
  void pop()
  {
    ms.pop();
  }
  
private:
  std::stack< std::vector<T> > ms;
};
  
  
} // language
} // service
} // cc
  
#endif //SERVICE_CPPSERVICEHELPER_SLICER_PDG_MULTISTACK_H