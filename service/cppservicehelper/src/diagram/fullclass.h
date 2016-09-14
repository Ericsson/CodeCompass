#ifndef FULLCLASS_H
#define FULLCLASS_H

#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class FullClass
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> subfiles;
  };

  FullClass(Relations relations, std::shared_ptr<odb::database> db)
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