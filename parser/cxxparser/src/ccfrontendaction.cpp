/*
 * macrofrontendaction.cpp
 *
 *  Created on: Jul 4, 2013
 *      Author: ezoltbo
 */

#include "cxxparser/cxxparser.h"

#include "ccfrontendaction.h"

#include "ccastconsumer.h"
#include "ccppcallbacks.h"

namespace cc
{
namespace parser
{
 
std::unique_ptr<clang::ASTConsumer> CcFrontendAction::CreateASTConsumer(
  clang::CompilerInstance& ci_, clang::StringRef inFile_)
{
  auto& pp = ci_.getPreprocessor();
  pp.addPPCallbacks(
    std::make_unique<CcPPCallbacks>(
      pp,
      ci_.getASTContext(),
      srcMgr,
      w,
      targetFile,
      cxxParser.getPersister()));

  return std::make_unique<CcAstConsumer>(w, parseProps, srcMgr, cxxParser);
}

} // parser
} // cc
