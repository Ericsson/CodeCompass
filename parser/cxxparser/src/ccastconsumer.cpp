/*
 * ccastconsumer.cpp
 *
 *  Created on: Jul 8, 2013
 *      Author: ezoltbo
 */

#include <map>
#include <unordered_set>

#include "ccastconsumer.h"

#include <clang/AST/Decl.h>

#include "model/workspace.h"

#include "assignmentcollector.h"
#include "clangastvisitor.h"
#include "cxxparsesession.h"
#include "nodeinfocollector.h"
#include "relationcollector.h"
#include "documentationcommentcollector.h"
#include "declcontextvisitor.h"
#include "metricscollector.h"

namespace cc
{
namespace parser
{
  
void CcAstConsumer::HandleTranslationUnit(clang::ASTContext& Ctx)
{
    clang::TranslationUnitDecl* trunit = Ctx.getTranslationUnitDecl();

    //visiting
    CxxParseSession s(Ctx);

    // the destructor of the visitor (saves some ast node) that is needed by nodeInfoCollector
    {
      ClangASTVisitor visitor(w, srcMgr, s, cxxParser.getPersister());
      visitor.TraverseDecl(trunit);
    }

    {
      AssignmentCollector assCollector(w, s);
      assCollector.TraverseDecl(trunit);
    }

    {
      NodeInfoCollector nodeInfoCollector(w, s, cxxParser.getPersister());
      nodeInfoCollector.TraverseDecl(trunit);
    }

    {
      RelationCollector relationCollector(srcMgr, s, cxxParser);
      relationCollector.TraverseDecl(trunit);
    }
    
    /*{
      DeclContextVisitor contextCollector(w, s, cxxParser.getPersister());
      contextCollector.TraverseDecl(trunit);
    }*/

    auto it = parseProps.options.find("skip-docparser");
    if (it == parseProps.options.end() || it->second == "false")
    {
      DocumentationCommentCollector documentationCommentCollector(w, s);
      documentationCommentCollector.TraverseDecl(trunit);
    }
    
    {
        MetricsCollector metricsCollector(w, Ctx, s, srcMgr);
        metricsCollector.TraverseDecl(trunit);
    }
}

} // parser
} // cc
