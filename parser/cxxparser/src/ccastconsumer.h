/*
 * ccastconsumer.h
 *
 *  Created on: Jul 8, 2013
 *      Author: ezoltbo
 */

#ifndef CCASTCONSUMER_H_
#define CCASTCONSUMER_H_

#include <unordered_set>

#include <clang/AST/ASTConsumer.h>
#include <clang/Lex/PPCallbacks.h>

#include "model/file.h"
#include "model/cxx/cpptype.h"
#include "parser/commondefs.h"

namespace cc
{
  
namespace model
{
  class Workspace;
}

namespace parser
{
  
class SourceManager;
class CXXParser;

class CcAstConsumer : public clang::ASTConsumer
{
public:
  CcAstConsumer(
    std::shared_ptr<model::Workspace> pw,
    const ParseProps& pprops,
    SourceManager& srcMgr,
    CXXParser& cxxParser_
  )
  : w(pw),
    parseProps(pprops),
    srcMgr(srcMgr),
    cxxParser(cxxParser_)
{}

  virtual void HandleTranslationUnit(clang::ASTContext& Ctx) override;

private:
  std::shared_ptr<model::Workspace> w;
  ParseProps parseProps;
  SourceManager &srcMgr;
  CXXParser& cxxParser;
};

} // parser
} // cc
#endif /* CCASTCONSUMER_H_ */
