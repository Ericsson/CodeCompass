#include "diagram.h"

namespace cc
{
namespace service
{
namespace competence
{

Diagram::Diagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _projectHandler(db_, datadir_, context_)
{
}

void Diagram::getDirectoryCompetenceDiagram(
  util::Graph& graph_,
  const core::AstNodeId& astNodeId_)
{

}
}
}
}