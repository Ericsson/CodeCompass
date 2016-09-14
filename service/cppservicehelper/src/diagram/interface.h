#ifndef INTERFACE_DIAGRAM_H
#define INTERFACE_DIAGRAM_H

#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class InterfaceDiagram
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> uses;
    std::function<std::vector<model::FileId>(const model::FileId&)> usedby;
    std::function<std::vector<model::FileId>(const model::FileId&)> provides;
    std::function<std::vector<model::FileId>(const model::FileId&)> providedby;
    std::function<std::vector<model::FileId>(const model::FileId&)> contains;
    std::function<std::vector<model::FileId>(const model::FileId&)> containedby;
  };

  InterfaceDiagram(Relations relations, std::shared_ptr<odb::database> db)
    : relations(relations), db(db) {}

  void buildDiagram(util::Graph& graph, const model::FileId& startNode);
  
private:
  Relations relations;
  std::shared_ptr<odb::database> db;
};

}
}
}

#endif