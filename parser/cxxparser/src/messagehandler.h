#ifndef CXXPARSER_CLANG_MESSAGE_HANDLER_H
#define CXXPARSER_CLANG_MESSAGE_HANDLER_H

#include <utility>
#include <deque>

#include <clang/Basic/Diagnostic.h>

#include <parser/sourcemanager.h>
#include <model/position.h>
#include <model/buildaction.h>
#include <model/buildlogmessage.h>

namespace cc
{
namespace parser
{

class MessageHandler : public clang::DiagnosticConsumer
{
public:
  struct Item
  {
    model::Range             fileLoc;
    std::string              filePath;
    model::BuildLogMessage   message;
    bool                     invalidLocation;
  };

  typedef std::deque<Item> MessageCont;

public:
  MessageHandler();

  void HandleDiagnostic(clang::DiagnosticsEngine::Level diagLevel_,
                        const clang::Diagnostic&        info_) override;

  void persistMessages(std::shared_ptr<model::Workspace>   workspace_,
                       model::BuildActionPtr               buildAction_,
                       SourceManager&                      srcman_);

private:
  typedef std::shared_ptr<MessageCont> MessageContPtr;

  MessageContPtr _messages;
};

} // parser
} // cc

#endif // CXXPARSER_CLANG_MESSAGE_HANDLER_H
