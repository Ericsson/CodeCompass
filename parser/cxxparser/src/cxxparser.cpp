#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <sstream>
#include <map>

#include <odb/transaction.hxx>
#include <odb/tracer.hxx>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/Version.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Options.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/Utils.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Preprocessor.h>


#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Host.h>

#include <util/util.h>
#include <parser/sourcemanager.h>
#include <parser/parser.h>
#include <parser/crashprotect.h>
#include <cxxparser/cxxparser.h>

#include <model/buildlog.h>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/cxx/cppentity.h>
#include <model/cxx/cppentity-odb.hxx>
#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>
#include <model/cxx/cppheaderinclusion.h>
#include <model/cxx/cppheaderinclusion-odb.hxx>
#include <model/buildtarget-odb.hxx>

#include "messagehandler.h"
#include "clangastvisitor.h"
#include "nodeinfocollector.h"
#include "ccfrontendaction.h"
#include "symbolhelper.h"
#include "parameters.h"
#include "model/diagram/edgeattribute.h"

#include <util/environment.h>
#include <util/streamlog.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <indexer-api/search_indexer_types.h>
#include <indexer-api/search_indexer_constants.h>
#include <indexer-api/IndexerService.h>

namespace cc
{
namespace parser
{

using namespace cc::model;
using CompilerPtr = std::unique_ptr<clang::CompilerInstance>;

CXXParser::CXXParser(std::shared_ptr<model::Workspace> w_):
  w(w_), astPersister(w_),
  _diagNodeCache(w_->getDb()),
  _diagEdgeCache(w->getDb()),
  _diagEdgeAttrCache(w->getDb())
{
  fillCaches();
}

void CXXParser::fillCaches()
{
  odb::database& db = *w->getDb();
  util::OdbTransaction transaction(db);

  // Make node-edge connections consistent after a crash
  transaction([&, this]{
    std::size_t deletedObjects = 0;

    // Delete edges with invalid node references
    deletedObjects = 0;
    for (const auto& edgeid : db.query<model::EdgeIds>())
    {
      if (!db.find<model::Node>(edgeid.from) ||
          !db.find<model::Node>(edgeid.to))
      {
        ++deletedObjects;
        db.erase<model::Edge>(edgeid.id);
      }
    }
    
    if (deletedObjects > 0)
    {
      SLog(util::STATUS)
        << deletedObjects << " inconsistent edge deleted!";
    }

    // Delete edge attributes with invalid node references
    deletedObjects = 0;
    for (const auto& eaid : db.query<model::EdgeAttributeIds>())
    {
      if (!db.find<model::Edge>(eaid.edge))
      {
        ++deletedObjects;
        db.erase<model::EdgeAttribute>(eaid.id);
      }
    }

    if (deletedObjects > 0)
    {
      SLog(util::STATUS)
        << deletedObjects << " inconsistent edge attribute deleted!";
    }
  });

  transaction([&, this]{
    for (const model::Node& node : db.query<model::Node>())
      _diagNodeCache.insertToCache(
        node.domainId + std::to_string(node.domain), node);

    for (const model::Edge& edge : db.query<model::Edge>())
      _diagEdgeCache.insertToCache(
        std::to_string(edge.from->id) +
        std::to_string(edge.to->id) +
        std::to_string(edge.type),
        edge);

    for (const model::EdgeAttribute& attr : db.query<model::EdgeAttribute>())
      _diagEdgeAttrCache.insertToCache(
        std::to_string(attr.edge->id) + attr.key + attr.value, attr);
  });
}

bool CXXParser::accept(const std::string& path_)
{
  return util::isCppFile(path_);
}

std::future<ParseResult> CXXParser::parse(
  ParseProps parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::string& path_,
  const std::string& targetPath_,
  const std::vector<std::string>& opts_)
{
  std::string target = targetPath_;
  if (target.empty())
  {
    target = getDefaultTargetPath(target);
  }

  // Add a dummy program name as compiler
  std::vector<std::string> options(opts_);
  options.insert(options.begin(), "CodeCompassParser");
  options.push_back(path_);

  model::FilePtr sourceFile, targetFile;
  srcMgr_.getCreateFile(path_, sourceFile);
  srcMgr_.getCreateFile(targetPath_, targetFile, SourceManager::NoContent);

  BuildSourceTargets bt{std::make_pair(sourceFile, targetFile)};
  return parse(parseProps_, buildAction_, srcMgr_, options, bt);
}

std::future<ParseResult> CXXParser::parse(
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::vector<std::string>& opts_,
  const BuildSourceTargets& files_)
{
  return Parser::getParser().getThreadPool().submit([=, &srcMgr_]()
  {
    return parseThread(parseProps_, buildAction_, srcMgr_, opts_, files_);
  });
}

void CXXParser::postParse(
  ParseProps parseProps_,
  std::size_t& numOfSuccess_,
  std::size_t& numOfFail_,
  search::IndexerServiceIf& indexerSvc_)
{
  numOfSuccess_ = _numOfSuccess;
  numOfFail_ = _numOfFail;

  if (_numOfSuccess || _numOfFail)
  {
    w->addStatistics("C++", "Successfully parsed actions", _numOfSuccess);
    w->addStatistics("C++", "Partially parsed actions", _numOfFail);
  }

  util::OdbTransaction trans(std::shared_ptr<odb::database>(w->getDb(),
    util::NoDelete()));

  // File id -> fields map
  std::map<FileId, search::Fields> docFields;

  trans([&, this]()
  {
    // Map from File id to its new parse status
    typedef std::unordered_map<FileId, File::ParseStatus> StatusMap;
    StatusMap newParseStatus;

    //--- Get built files ---//

    for (const CxxBuildSources& source : w->getDb()->query<CxxBuildSources>())
      newParseStatus[source.file] = File::PSFullyParsed;

    //--- Get included files ---//

    for (const IncludedHeaders& header : w->getDb()->query<IncludedHeaders>())
      newParseStatus[header.file] = File::PSFullyParsed;

    //--- Set partially parsed files ---//

    typedef odb::query<BuildLogFileMessageType> BmQuery;
    typedef odb::result<BuildLogFileMessageType> BmResult;

    BmResult bmTypes(w->getDb()->query<BuildLogFileMessageType>(
      BmQuery(BmQuery::BuildLog::location.file.is_not_null())));

    for (const BuildLogFileMessageType& bmType : bmTypes)
    {
      if (!bmType.messageType)
        continue;

      FileId astFileId = bmType.fileId.get();

      BuildLogMessage::MessageType mt = bmType.messageType.get();
      if (mt == BuildLogMessage::Error ||
          mt == BuildLogMessage::FatalError ||
          mt == BuildLogMessage::Unknown)
      {
        newParseStatus[astFileId] = File::PSPartiallyParsed;
      }
    }

    //--- Update parse status in database ---//

    for (const auto& status : newParseStatus)
    {
      std::shared_ptr<File> astFile = w->getDb()->load<File>(status.first);

      if (astFile->parseStatus != status.second &&
          astFile->parseStatus != File::PSPartiallyParsed)
      {
        // Set new (changed) parse status but keep the original if it is
        // partially parsed.
        astFile->parseStatus = status.second;
        w->getDb()->update(astFile);

        search::FieldValue value;
        switch (status.second)
        {
          case File::PSFullyParsed:
            value.value =
              search::g_search_indexer_constants.PSTATUS_PARSED;
            break;
          case File::PSPartiallyParsed:
            value.value =
              search::g_search_indexer_constants.PSTATUS_PART_PARSED;
            break;
          default:
            value.value =
              search::g_search_indexer_constants.PSTATUS_NOT_PARSED;
            break;
        }

        docFields
          [status.first]
          [search::g_search_indexer_constants.FIELD_PARSE_STATUS]
          .emplace_back(std::move(value));
      }
    }
  });

  if (!parseProps_.options.count("no-search-ast-info"))
  {
    SLog(util::STATUS)
      << "Adding extra search informations based on C++ AST";

    // CppEntity.typeid -> tag type map
    std::map<std::string, std::string> entityCtagsType = {
      { "cc::model::CppEnum",           "Type"        },
      { "cc::model::CppEnumConstant",   "Constant"    },
      { "cc::model::CppFunction",       "Function"    },
      { "cc::model::CppMacro",          "Macro"       },
      { "cc::model::CppType",           "Type"        },
      { "cc::model::CppTypedef",        "Type"        },
      { "cc::model::CppVariable",       "Variable"    },
      { "cc::model::CppFunctionPointer","Variable"    }
    };

    trans([&, this]()
    {
      typedef odb::query<CppQualifiedName> QnQuery;
      typedef odb::result<CppQualifiedName> QnResult;

      QnResult qnames(w->getDb()->query<CppQualifiedName>(
        QnQuery(QnQuery::CppAstNode::location.file.is_not_null())));
      for (const CppQualifiedName& qname : qnames)
      {
        search::FieldValue value;

        value.location.startLine = qname.start_line;
        value.location.startColumn = qname.start_col;
        value.location.endLine = qname.end_line;
        value.location.endColumn = qname.end_col;
        value.value = qname.qualifiedName;
        value.context = entityCtagsType.at(qname.typeId);
        value.__isset.location = true;
        value.__isset.context = true;

        FileId fid = qname.file.get();
        docFields[fid][search::g_search_indexer_constants.FIELD_DEFINITIONS]
          .emplace_back(std::move(value));
      }
    });

    for (const auto& doc : docFields)
    {
      indexerSvc_.addFieldValues(std::to_string(doc.first), doc.second);
    }
  }
  else
  {
    SLog(util::STATUS)
     << "Skip adding extra search informations based on C++ AST";
  }
}

std::string CXXParser::getDefaultTargetPath(const std::string& srcPath_)
{
  return util::getPathAndFileWithoutExtension(srcPath_) + ".o";
}

ParseResult CXXParser::parseThread(
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::vector<std::string>& opts_,
  const BuildSourceTargets& files_)
{
  std::vector<CompilerArgments> compArgsVector;
  {
    MessageHandler msgHandler;
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagIds(
      new clang::DiagnosticIDs());
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts(
      new clang::DiagnosticOptions());
    llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagEng(
      new clang::DiagnosticsEngine(diagIds, diagOpts.get(), &msgHandler, false));

    clang::ProcessWarningOptions(*diagEng, *diagOpts);

    compArgsVector = processCommandLine(opts_, *diagEng);

    util::OdbTransaction trans(*w->getDb());
    trans([this, buildAction_, &srcMgr_, &msgHandler]
    {
      msgHandler.persistMessages(w, buildAction_, srcMgr_);
    });
  }

  if (compArgsVector.empty())
  {
    SLog(cc::util::ERROR) << "Failed to resolve compiler arguments!";
    return PARSE_FAIL;
  }

  ParseResult finalResult = PARSE_SUCCESS;
  for (CompilerArgments& compArgs : compArgsVector)
  {
    ParseResult result = parseTranslationUnit(parseProps_, buildAction_,
      srcMgr_, compArgs, files_);
    if (result == PARSE_FAIL)
    {
      finalResult = result;
    }
  }

  util::OdbTransaction trans(*w->getDb());
  trans([this, buildAction_]
  {
    buildAction_->state = model::BuildAction::StParsed;
    w->getDb()->update(*buildAction_);
  });

  return finalResult;
}

ParseResult CXXParser::parseTranslationUnit(
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::vector<std::string>& opts_,
  const BuildSourceTargets& files_)
{
  util::OdbTransaction trans(*w->getDb());
  return trans([&, this]
  {
    std::string realCommandLine;
    {
      std::stringstream ss;

      ss << "Command line argument vector:" << std::endl;
      std::copy(opts_.begin(), opts_.end(),
        std::ostream_iterator<std::string>(ss, "\n"));

      realCommandLine = ss.str();
    }

    crash::ScopedProtection crashProt(buildAction_, realCommandLine);

    MessageHandler msgHandler;
    bool parsed = parseFile(msgHandler, parseProps_, buildAction_, srcMgr_,
      opts_, files_);
    msgHandler.persistMessages(w, buildAction_, srcMgr_);

    ParseResult res = PARSE_FAIL;
    if (parsed && !msgHandler.getNumErrors())
    {
      ++_numOfSuccess;
      res = PARSE_SUCCESS;
    }
    else
    {
      ++_numOfFail;
      res = PARSE_FAIL;
    }

    return res;
  });
}

bool CXXParser::parseFile(
  clang::DiagnosticConsumer& diagConsumer_,
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::vector<std::string>& options_,
  const BuildSourceTargets& files_)
{
  using namespace clang;

  // Create compile and diagnostics
  CompilerPtr compiler = std::make_unique<clang::CompilerInstance>();
  compiler->createDiagnostics(&diagConsumer_, false);
  if (!compiler->hasDiagnostics())
  {
    SLog(cc::util::ERROR) << "hasDiagnostics returned false!";
    return false;
  }

  // Fill up compiler arguments vector
  std::vector<const char*> arguments;
  for (const std::string& option : options_)
  {
    arguments.push_back(option.c_str());
  }

  // Create invocation
  CompilerInvocation::CreateFromArgs(
    compiler->getInvocation(),
    arguments.data(),
    arguments.data() + arguments.size(),
    compiler->getDiagnostics());

  auto pAction = compiler->getInvocation().getFrontendOpts().ProgramAction;
  if (pAction != frontend::ParseSyntaxOnly)
  {
    SLog(cc::util::WARNING) << "Skipping program action: " << pAction;
    return true;
  }

  // Workaround if there is an option which can be recognized as an input
  compiler->getFrontendOpts().Inputs.resize(1);

  // Get current source file from db
  std::string currentSource = compiler->getFrontendOpts().
    Inputs.front().getFile();
  FilePtr source;
  if (!srcMgr_.getCreateFile(currentSource, source))
  {
    SLog(cc::util::ERROR)
      << "Failed to add file '"
      << currentSource
      << "' to the database!";
    return false;
  }

  // Set target translation unit
  FilePtr target;
  for (const auto& srcToTarg: files_)
  {
    if (srcToTarg.first->id == source->id)
    {
      target = srcToTarg.second;
      break;
    }
  }

  if (!target)
  {
    SLog(cc::util::WARNING)
      << "Target not found for '" << currentSource << "'!";
    if (!srcMgr_.getCreateFile("/unknown_target", target,
        SourceManager::NoContent))
    {
      SLog(cc::util::ERROR) << "Failed to add fall-back target!";
      return false;
    }
  }

  CcFrontendAction frontendAction(w, target, parseProps_, srcMgr_, *this);
  std::unique_ptr<clang::ASTUnit> AST(
    clang::ASTUnit::LoadFromCompilerInvocationAction(
      &compiler->getInvocation(),
      &compiler->getDiagnostics(),
      &frontendAction));
  if (AST)
  {
    return true;
  }

  SLog(cc::util::ERROR) << "Failed to parser '" << currentSource << "'!";
  return false;
}

} // parser
} // cc
