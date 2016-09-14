#ifndef SUBSYSTEMDEPENDENCY_H
#define	SUBSYSTEMDEPENDENCY_H

#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class SubsystemDependency
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> subdir;
    std::function<std::vector<model::FileId>(const model::FileId&)> implements;
    std::function<std::vector<model::FileId>(const model::FileId&)> depends;
  };

  SubsystemDependency(Relations relations, std::shared_ptr<odb::database> db)
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