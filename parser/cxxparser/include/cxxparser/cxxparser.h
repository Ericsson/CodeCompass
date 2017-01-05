#ifndef CXXPARSER_CXXPARSER_H
#define CXXPARSER_CXXPARSER_H

#include <vector>
#include <map>
#include <string>
#include <unordered_set>

#include <fileparser/fileparser.h>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cpptype.h>
#include <model/workspace.h>
#include <model/diagram/node.h>
#include <model/diagram/node-odb.hxx>
#include <model/diagram/edge.h>
#include <model/diagram/edge-odb.hxx>
#include <model/diagram/edgeattribute.h>
#include <model/diagram/edgeattribute-odb.hxx>
#include <util/odbobjectcache.h>

#include "cxxastpersister.h"

namespace clang
{
  class ASTUnit;
  class CompilerInstance;
  class DiagnosticsEngine;
  class DiagnosticConsumer;
}

namespace cc
{
namespace parser
{
  
  
class SourceManager;

typedef std::map<std::string, model::CppTypePtr> TypeMap;

typedef util::OdbObjectCache<std::string, model::Node> NodeCache;
typedef util::OdbObjectCache<std::string, model::Edge> EdgeCache;
typedef util::OdbObjectCache<std::string, model::EdgeAttribute> EdgeAttrCache;

class CXXParser : public IFileParser
{
public:
  enum ParseFlag
  {
    PFRestarted = 1
  };

  CXXParser(std::shared_ptr<model::Workspace> w_);

  virtual bool accept(const std::string& path_) override;

  virtual std::future<ParseResult> parse(
    ParseProps parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::string& path_,
    const std::string& targetpath_,
    const std::vector<std::string>& opts_) override;
  
  virtual std::future<ParseResult> parse(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets& files_) override;

  virtual void postParse(
    ParseProps parseProps_,
    std::size_t& numOfSuccess_,
    std::size_t& numOfFail_,
    search::IndexerServiceIf& indexerSvc_) override;

  std::string getDefaultTargetPath(const std::string& srcPath_) override;
  
  CxxAstPersister&  getPersister()      { return astPersister; }

  NodeCache& getNodeCache() { return _diagNodeCache; }
  EdgeCache& getEdgeCache() { return _diagEdgeCache; }
  EdgeAttrCache& getEdgeAttrCache() { return _diagEdgeAttrCache; }
  
private:
  void fillCaches();
  
  ParseResult parseThread(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets& files_);

  ParseResult parseTranslationUnit(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets& files_);

  bool parseFile(
    clang::DiagnosticConsumer& diagConsumer_,
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets& files_);

  std::shared_ptr<model::Workspace> w;
  CxxAstPersister astPersister;
  
  NodeCache _diagNodeCache;
  EdgeCache _diagEdgeCache;
  EdgeAttrCache _diagEdgeAttrCache;
};

} //parser
} //cc

#endif //CXXPARSER_PARSER_H
