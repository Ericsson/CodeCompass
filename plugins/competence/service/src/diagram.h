#ifndef CODECOMPASS_DIAGRAM_H
#define CODECOMPASS_DIAGRAM_H

#include <service/competenceservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

namespace cc
{
namespace service
{
namespace competence
{

class Diagram
{
public:
  Diagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getDirectoryCompetenceDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

private:
  core::ProjectServiceHandler _projectHandler;

};
}
}
}





#endif //CODECOMPASS_DIAGRAM_H
