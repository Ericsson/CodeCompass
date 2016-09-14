#ifndef EXTERNALUSERS_H
#define	EXTERNALUSERS_H

#include <memory>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

class ExternalUsers
{
public:
  struct Relations
  {
    std::function<std::vector<model::FileId>(const model::FileId&)> subdir;
    std::function<std::vector<model::FileId>(const model::FileId&)> implementedby;
    std::function<std::vector<model::FileId>(const model::FileId&)> dependenton;
  };
  
  ExternalUsers(Relations relations, std::shared_ptr<odb::database> db)
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