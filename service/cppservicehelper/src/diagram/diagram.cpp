#include <algorithm>
#include <map>

#include "model/cxx/cppastnode.h"
#include "model/cxx/cppastnode-odb.hxx"
#include "model/cxx/cppheaderinclusion.h"
#include "model/cxx/cppheaderinclusion-odb.hxx"
#include "model/diagram/edge.h"
#include "model/diagram/edge-odb.hxx"
#include "model/diagram/node.h"
#include "model/diagram/node-odb.hxx"

#include "includedependency.h"
#include "usedcomponents.h"
#include "componentusers.h"
#include "interface.h"
#include "subsystemdependency.h"
#include "externaldependency.h"
#include "externalusers.h"

#include <util/odbtransaction.h>
#include <util/util.h>

#include "diagram.h"
#include "model/cxx/cppproviderelation.h"
#include "externaldependency.h"

namespace cc {
namespace service {
namespace language {

struct Include
{
  Include(std::shared_ptr<odb::database> db)
    : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> include;

    transaction([&, this]{
      IncludeResult res = db->query<model::CppHeaderInclusion>(
        IncludeQuery::includer == fileId);

      for (const auto& inclusion : res)
        include.push_back(inclusion.included.object_id());
    });

    return include;
  }

private:
  typedef odb::query<model::CppHeaderInclusion> IncludeQuery;
  typedef odb::result<model::CppHeaderInclusion> IncludeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevInclude
{
  RevInclude(std::shared_ptr<odb::database> db)
    : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> include;

    transaction([&, this]{
      IncludeResult res = db->query<model::CppHeaderInclusion>(
        IncludeQuery::included == fileId);

      for (const auto& inclusion : res)
        include.push_back(inclusion.includer.object_id());
    });

    return include;
  }

private:
  typedef odb::query<model::CppHeaderInclusion> IncludeQuery;
  typedef odb::result<model::CppHeaderInclusion> IncludeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Use
{
  Use(std::shared_ptr<odb::database> db)
    :db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> use;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::from->domainId == std::to_string(fileId) &&
        EdgeQuery::type == model::Edge::USE);
      
      for (const model::Edge& edge : res)
        use.push_back(std::stoull(edge.to->domainId));
    });
    
    return use;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevUse
{
  RevUse(std::shared_ptr<odb::database> db)
    :db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> use;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::to->domainId == std::to_string(fileId) &&
        EdgeQuery::type == model::Edge::USE);
      
      for (const model::Edge& edge : res)
        use.push_back(std::stoull(edge.from->domainId));
    });
    
    return use;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Provide
{
  Provide(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> provide;

    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::from->domainId == std::to_string(fileId) &&
        EdgeQuery::type == model::Edge::PROVIDE);
      
      for (const model::Edge& relation : res)
        provide.push_back(std::stoull(relation.to->domainId));
    });
    
    return provide;
  }

private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevProvide
{
  RevProvide(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> provide;

    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::to->domainId == std::to_string(fileId) &&
        EdgeQuery::type == model::Edge::PROVIDE);
      
      for (const model::Edge& relation : res)
        provide.push_back(std::stoull(relation.from->domainId));
    });
    
    return provide;
  }

private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Contain
{
  Contain(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> contain;

    transaction([&, this]{
      TargetResult targets = db->query<model::BuildTarget>(
        TargetQuery::file == fileId);

      for (const model::BuildTarget& target : targets)
        for (const auto& source : target.action->sources)
          contain.push_back(source.lock().load()->file.object_id());
    });

    return contain;
  }

private:
  typedef odb::query<model::BuildTarget> TargetQuery;
  typedef odb::result<model::BuildTarget> TargetResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevContain
{
  RevContain(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}

  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> contain;

    transaction([&, this]{
      SourceResult sources = db->query<model::BuildSource>(
        SourceQuery::file == fileId);

      for (const model::BuildSource& source : sources)
        for (const auto& target : source.action->targets)
          contain.push_back(target.lock().load()->file.object_id());
    });

    return contain;
  }

private:
  typedef odb::query<model::BuildSource> SourceQuery;
  typedef odb::result<model::BuildSource> SourceResult;

  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Subdir
{
  Subdir(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> subdirs;
    
    transaction([&, this]{
      FileResult sub = db->query<model::File>(
        FileQuery::parent == fileId &&
        FileQuery::type == model::File::Directory);
      
      for (const model::File& subdir : sub)
        subdirs.push_back(subdir.id);
    });
    
    return subdirs;
  }
  
private:
  typedef odb::query<model::File> FileQuery;
  typedef odb::result<model::File> FileResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Descendant
{
  Descendant(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> result;
    
    transaction([&, this]{
      std::string path = db->load<model::File>(fileId)->path;
      FileResult descendants = db->query<model::File>(
        FileQuery::path + SQL_ILIKE + FileQuery::_val(path + '%'));
      
      std::transform(
        descendants.begin(),
        descendants.end(),
        std::back_inserter(result),
        [](const model::File& file){ return file.id; });
    });
    
    return result;
  }
  
private:
  typedef odb::query<model::File> FileQuery;
  typedef odb::result<model::File> FileResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Implements
{
  Implements(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> result;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::from->domainId == std::to_string(fileId) &&
        EdgeQuery::from->domainId != EdgeQuery::to->domainId &&
        EdgeQuery::type == model::Edge::IMPLEMENT);
      
      for (const model::Edge& relation : res)
        result.push_back(std::stoull(relation.to->domainId));
    });
    
    return result;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevImplements
{
  RevImplements(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> result;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::to->domainId == std::to_string(fileId) &&
        EdgeQuery::to->domainId != EdgeQuery::from->domainId &&
        EdgeQuery::type == model::Edge::IMPLEMENT);
      
      for (const model::Edge& relation : res)
        result.push_back(std::stoull(relation.from->domainId));
    });
    
    return result;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct Depends
{
  Depends(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> result;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::from->domainId == std::to_string(fileId) &&
        EdgeQuery::from->domainId != EdgeQuery::to->domainId &&
        EdgeQuery::type == model::Edge::DEPEND);
      
      for (const model::Edge& relation : res)
        result.push_back(std::stoull(relation.to->domainId));
    });
    
    return result;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

struct RevDepends
{
  RevDepends(std::shared_ptr<odb::database> db) : db(db), transaction(db) {}
  
  std::vector<model::FileId> operator()(const model::FileId& fileId)
  {
    std::vector<model::FileId> result;
    
    transaction([&, this]{
      EdgeResult res = db->query<model::Edge>(
        EdgeQuery::to->domainId == std::to_string(fileId) &&
        EdgeQuery::to->domainId != EdgeQuery::from->domainId &&
        EdgeQuery::type == model::Edge::DEPEND);
      
      for (const model::Edge& relation : res)
        result.push_back(std::stoull(relation.from->domainId));
    });
    
    return result;
  }
  
private:
  typedef odb::query<model::Edge> EdgeQuery;
  typedef odb::result<model::Edge> EdgeResult;
  
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
};

void includeDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::IncludeDependency::Relations relations = {
    Include(db),
    RevInclude(db)
  };

  diagram::IncludeDependency diagramCreator(relations, db);

  diagramCreator.buildDiagram(graph, startNode);
}

void usedComponents(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::UsedComponents::Relations relations = {
    Include(db),
    RevProvide(db),
    RevContain(db)
  };

  diagram::UsedComponents diagramCreator(relations, db);

  diagramCreator.buildDiagram(graph, startNode);
}

void componentUsers(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::ComponentUsers::Relations relations = {
    RevInclude(db),
    Provide(db),
    RevContain(db)
  };

  diagram::ComponentUsers diagramCreator(relations, db);

  diagramCreator.buildDiagram(graph, startNode);
}

void interfaceDiagram(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::InterfaceDiagram::Relations relations = {
    Use(db),
    RevUse(db),
    Provide(db),
    RevProvide(db),
    Contain(db),
    RevContain(db)
  };

  diagram::InterfaceDiagram diagramCreator(relations, db);

  diagramCreator.buildDiagram(graph, startNode);
}

void subsystemDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::SubsystemDependency::Relations relations = {
    Subdir(db),
    Implements(db),
    Depends(db)
  };
  
  diagram::SubsystemDependency diagramCreator(relations, db);
  
  diagramCreator.buildDiagram(graph, startNode);
}

void externalDependency(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::ExternalDependency::Relations relations = {
    Subdir(db),
    Implements(db),
    Depends(db)
  };
  
  diagram::ExternalDependency diagramCreator(relations, db);
  
  diagramCreator.buildDiagram(graph, startNode);
}

void externalUsers(
  util::Graph& graph,
  const model::FileId& startNode,
  std::shared_ptr<odb::database> db)
{
  diagram::ExternalUsers::Relations relations = {
    Subdir(db),
    RevImplements(db),
    RevDepends(db)
  };
  
  diagram::ExternalUsers diagramCreator(relations, db);
  
  diagramCreator.buildDiagram(graph, startNode);
}

}
}
}
