#include <llvm/ADT/SmallString.h>
#include <cppparser/filelocutil.h>
#include "diagnosticmessagehandler.h"

namespace cc
{
namespace parser
{

DiagnosticMessageHandler::DiagnosticMessageHandler(
  llvm::raw_ostream& os,
  clang::DiagnosticOptions* diags,
  SourceManager& srcMgr_,
  std::shared_ptr<odb::database> db_)
    : TextDiagnosticPrinter(os, diags), _srcMgr(srcMgr_), _db(db_)
{
}

void DiagnosticMessageHandler::HandleDiagnostic(
  clang::DiagnosticsEngine::Level diagLevel_,
  const clang::Diagnostic& info_)
{
  clang::TextDiagnosticPrinter::HandleDiagnostic(diagLevel_, info_);

  model::BuildLog buildLog;

  //--- Message type ---//

  switch (diagLevel_)
  {
    case clang::DiagnosticsEngine::Note:
      buildLog.log.type = model::BuildLogMessage::Note;
      break;
    case clang::DiagnosticsEngine::Warning:
      buildLog.log.type = model::BuildLogMessage::Warning;
      break;
    case clang::DiagnosticsEngine::Error:
      buildLog.log.type = model::BuildLogMessage::Error;
      break;
    case clang::DiagnosticsEngine::Fatal:
      buildLog.log.type = model::BuildLogMessage::FatalError;
      break;
  }

  //--- Message content ---//

  llvm::SmallString<512> content;
  info_.FormatDiagnostic(content);
  buildLog.log.message = content.c_str();

  //--- Message location ---//

  if (info_.getLocation().isValid() && info_.hasSourceManager())
  {
    FileLocUtil fileLocUtil(info_.getSourceManager());
    const clang::SourceLocation& loc = info_.getLocation();

    model::Range fileLoc;
    fileLocUtil.setRange(loc, loc, fileLoc);

    buildLog.location.range = fileLoc;
    buildLog.location.file = _srcMgr.getFile(fileLocUtil.getFilePath(loc));
  }

  _messages.push_back(buildLog);
}

DiagnosticMessageHandler::~DiagnosticMessageHandler()
{
  util::OdbTransaction{_db}([this](){
    for (model::BuildLog& message : _messages)
      _db->persist(message);
  });
}

}
}
