/*
 * cxxparsesession.h
 *
 *  Created on: Apr 23, 2014
 *      Author: ezoltbo
 */

#ifndef CXXPARSESESSION_H_
#define CXXPARSESESSION_H_

#include <map>
#include <unordered_set>

#include <clang/AST/ASTContext.h>

#include <model/cxx/cppastnode.h>

namespace cc
{
namespace parser
{

struct CxxParseSession
{
  typedef std::map<const void*, model::CppAstNodePtr> PointerAstMap;
  typedef std::unordered_set<const void*> PointerSet;

  CxxParseSession(clang::ASTContext &ctx) :
    astContext(ctx)
  {
  }

  clang::ASTContext& astContext;
  PointerAstMap      clang2our;
  PointerSet         newNodes;
};

} // parser
} // cc

#endif /* CXXPARSESESSION_H_ */
