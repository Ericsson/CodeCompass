#ifndef CPPLANGUAGESERVICE_DIAGRAM
#define CPPLANGUAGESERVICE_DIAGRAM

#include <util/graph.h>

namespace cc {
namespace service {
namespace language {

void includeDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void usedComponents(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void componentUsers(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void interfaceDiagram(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void subsystemDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void externalDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

void externalUsers(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db);

}
}
}

#endif