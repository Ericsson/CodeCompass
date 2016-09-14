/* 
 * File:   cpppointeranalysis.h
 * Author: ecsomar
 *
 * Created on September 16, 2015, 9:20 AM
 */

#ifndef CPPPOINTERANALYSIS_H
#define	CPPPOINTERANALYSIS_H

#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{
  enum class UnaryOperator
  {
    And = 0, // &
    Start, // *
  };

#pragma db object
struct CppPointerAnalysis
{
  #pragma db id auto
  int id; /**< index of To of From's base classes */

  /**
   * It's store mangledNameHash of CppAstNode
   * At construct call/template call it stores the id of CppAstNode
   */
  unsigned long long lhs = 0;
  
  /**
   * It's store mangledNameHash of CppAstNode
   * At construct call/template call it stores the id of CppAstNode
   */
  unsigned long long rhs = 0;

  std::string lhsOperators; 
  
  std::string rhsOperators;
  
  bool operator==(const CppPointerAnalysis& other) const { return id == other.id; }
  
#ifndef NO_INDICES
  #pragma db index member(lhs)
  #pragma db index member(rhs)
#endif
};

#pragma db view
struct GetAllStatements {
  int id;
  unsigned long long lhs;
  unsigned long long rhs;
  std::string lhsOperators;  
  std::string rhsOperators;
};

#pragma db view
struct PointerAnalysisStatements {
  unsigned long long  statements;
};


#pragma db view object(CppPointerAnalysis)
struct CppPointerAnalysisCount
{
  #pragma db column("count(" + CppPointerAnalysis::id + ")")
  std::size_t count;
};


} // model
} // cc

#endif	/* CPPPOINTERANALYSIS_H */

