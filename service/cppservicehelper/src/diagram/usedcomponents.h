#ifndef USED_COMPONENTS_H
#define USED_COMPONENTS_H

#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class UsedComponents
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> uses;
    std::function<std::vector<model::FileId>(const model::FileId&)> providedby;
    std::function<std::vector<model::FileId>(const model::FileId&)> containedby;
  };

  UsedComponents(Relations relations, std::shared_ptr<odb::database> db)
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