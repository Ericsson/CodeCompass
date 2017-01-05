/*
 * macrofrontendaction.h
 *
 *  Created on: Jul 4, 2013
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_MACROFRONTENDACTION_H_
#define CODECOMPASS_MACROFRONTENDACTION_H_

#include <functional>
#include <unordered_set>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Lex/Preprocessor.h>

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

class CcFrontendAction : public clang::ASTFrontendAction
{
public:
  CcFrontendAction(
    std::shared_ptr<model::Workspace> pw,
    model::FilePtr targetFile_,
    const ParseProps& pprops,
    SourceManager& srcMgr,
    CXXParser& cxxParser_
    )
    : w(pw),
      targetFile(targetFile_),
      parseProps(pprops),
      srcMgr(srcMgr),
      cxxParser(cxxParser_)
  {}

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance& ci_,
    clang::StringRef inFile_) override;

private:
  std::shared_ptr<model::Workspace> w;
  model::FilePtr targetFile;
  ParseProps parseProps;
  SourceManager &srcMgr;
  CXXParser& cxxParser;
};

} // parser
} // cc
#endif /* MACROFRONTENDACTION_H_ */
