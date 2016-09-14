#ifndef INCLUDE_DEPENDENCY_H
#define INCLUDE_DEPENDENCY_H

#include <vector>
#include <functional>
#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class IncludeDependency
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> includes;
    std::function<std::vector<model::FileId>(const model::FileId&)> included;
  };

  IncludeDependency(Relations relations, std::shared_ptr<odb::database> db)
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