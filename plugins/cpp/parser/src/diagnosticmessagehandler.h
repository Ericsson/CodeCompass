#ifndef CC_PARSER_DIAGNOSTICMESSAGEHANDLER_H
#define CC_PARSER_DIAGNOSTICMESSAGEHANDLER_H

#include <vector>
#include <clang/Basic/Diagnostic.h>
#include <model/buildlog-odb.hxx>
#include <parser/sourcemanager.h>

namespace cc
{
namespace parser
{

class DiagnosticMessageHandler : public clang::DiagnosticConsumer
{
public:
  DiagnosticMessageHandler(
    SourceManager& srcMgr_,
    std::shared_ptr<odb::database> db_);
  ~DiagnosticMessageHandler();

  void HandleDiagnostic(
    clang::DiagnosticsEngine::Level diagLevel_,
    const clang::Diagnostic& info_) override;

private:
  SourceManager& _srcMgr;
  std::vector<model::BuildLog> _messages;
  std::shared_ptr<odb::database> _db;
};

}
}

#endif
