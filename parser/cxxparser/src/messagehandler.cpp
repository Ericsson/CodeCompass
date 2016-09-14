#include "messagehandler.h"

#include <llvm/ADT/SmallString.h>

#include <cxxparser/internal/filelocutilbase.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include <model/workspace.h>
#include <model/file-odb.hxx>
#include <model/fileloc-odb.hxx>
#include <model/buildaction-odb.hxx>
#include <model/buildlog-odb.hxx>
#include <model/buildlogmessage-odb.hxx>

namespace cc
{
namespace parser
{

MessageHandler::MessageHandler() :
  _messages(new MessageHandler::MessageCont)
{
}

void
MessageHandler::HandleDiagnostic(clang::DiagnosticsEngine::Level diagLevel_,
                                 const clang::Diagnostic&        info_)
{
  clang::DiagnosticConsumer::HandleDiagnostic(diagLevel_, info_);

  Item item;

  // Set message type
  switch (diagLevel_)
  {
    case clang::DiagnosticsEngine::Note:
      item.message.type = model::BuildLogMessage::Note;
      break;
    case clang::DiagnosticsEngine::Warning:
      item.message.type = model::BuildLogMessage::Warning;
      break;
    case clang::DiagnosticsEngine::Error:
      item.message.type = model::BuildLogMessage::Error;
      break;
    case clang::DiagnosticsEngine::Fatal:
      item.message.type = model::BuildLogMessage::FatalError;
      break;
    default:
      SLog(util::CRITICAL) << "Unknown diagnostic level!";
      item.message.type = model::BuildLogMessage::Unknown;
      break;
  }

  // Set message content
  {
    llvm::SmallString<512> mcontent;
    info_.FormatDiagnostic(mcontent);
    item.message.message = mcontent.c_str();
  }

  SLog() << "Diagnostic message: " << item.message.message;

  item.invalidLocation = true;

  // Set location
  if (info_.getLocation().isValid())
  {
    if (info_.hasSourceManager())
    {
      item.invalidLocation = false;

      FileLocUtilBase flocUtil(info_.getSourceManager());
      const clang::SourceLocation& loc = info_.getLocation();

      if (!flocUtil.setPosition(loc, item.fileLoc))
      {
        SLog(util::WARNING) << "Invalid location!";
        item.invalidLocation = true;
      }

      if (!flocUtil.getFilePath(loc, item.filePath))
      {
        SLog(util::WARNING) << "Invalid file path! : "<<item.filePath ;
        item.invalidLocation = true;
      }
    }
    else
    {
      SLog(util::WARNING) << "SourceManager not set!";
    }
  }

  _messages->push_back(item);
}

void MessageHandler::persistMessages(
  std::shared_ptr<model::Workspace>   workspace_,
  model::BuildActionPtr               buildAction_,
  SourceManager&                      srcman_)
{
  util::OdbTransaction trans(*workspace_->getDb());
  trans([&, this]()
  {
    SLog() << "Persist messages";

    for (auto item : (*_messages))
    {
      if (item.message.message.find("unknown argument") == 0)
      {
        SLog() << "Skipping 'unknown argument' message";
        continue;
      }

      model::BuildLog log;

      log.action = buildAction_;
      log.log = item.message;

      SLog() << "Persist message: " << log.log.message;

      // If location is valid set it
      if (!item.invalidLocation)
      {
        model::FileLoc loc;

        if (srcman_.getCreateFile(item.filePath, loc.file))
        {
          loc.range = item.fileLoc;

          log.location = loc;
        }
        else
        {
          // Location will be set to NULL in DB
          SLog(util::WARNING) << "getCreateFile failed!";
        }
      }

      workspace_->getDb()->persist(log);
    }
  });
}

} // parser
} // cc
