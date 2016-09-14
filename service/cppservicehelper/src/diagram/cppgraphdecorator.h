#ifndef CPP_GRAPH_DECORATOR_H
#define CPP_GRAPH_DECORATOR_H

#include <memory>
#include <map>
#include <util/diagram/graphdecorator.h>
#include <util/graph.h>
#include <util/util.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"
#include "model/diagram/edgeattribute.h"
#include "model/diagram/edgeattribute-odb.hxx"

namespace cc {
namespace service {
namespace diagram {

enum class CppEdgeType {
  USES,
  USEDBY,
  PROVIDES,
  PROVIDEDBY,
  CONTAINS,
  CONTAINEDBY,
  SUBDIR,
  IMPLEMENTS,
  IMPLEMENTEDBY,
  DEPENDS,
  DEPENDENTON
};

class CppGraphDecorator
  : public util::diagram::GraphDecorator<model::FileId, CppEdgeType>
{
public:
  CppGraphDecorator(std::shared_ptr<odb::database> db_, util::Graph& graph_)
    : util::diagram::GraphDecorator<model::FileId, CppEdgeType>(graph_),
      _db(db_), _transaction(db_) {}

  std::map<std::string, std::string> decorateNode(
    const model::FileId& fileId)
  {
    std::map<std::string, std::string> result;
    
    model::File file = getFile(fileId);

    result["id"]      = std::to_string(file.id);
    result["label"]   = getLastNParts(file.path, 3);
    result["tooltip"] = file.path;

    switch (util::getFileType(file.path))
    {
      case util::FileType::BINARY:
        result["shape"] = "box3d";
        result["fillcolor"] = "orange";
        break;

      case util::FileType::SOURCE:
        result["shape"] = "box";
        result["fillcolor"] = "#afcbe4";
        break;

      case util::FileType::HEADER:
        result["fillcolor"] = "#e4afaf";
        break;

      case util::FileType::OBJECT:
        result["shape"] = "folder";
        result["fillcolor"] = "#afe4bf";
        break;

      case util::FileType::UNKNOWN:
        result["fillcolor"] = "white";
        break;
    }

    if (result.find("fillcolor") != result.end())
      result["style"] = "filled";
    
    return result;
  }
  
  std::map<std::string, std::string> decorateEdge(
    const CppEdgeType& edgeType)
  {
    std::map<std::string, std::string> result;
    
    switch (edgeType)
    {
      case CppEdgeType::USES:
        result["label"] = "uses";
        break;

      case CppEdgeType::USEDBY:
        result["label"] = "used by";
        break;

      case CppEdgeType::PROVIDES:
        result["label"] = "provides";
        result["color"] = "red";
        break;

      case CppEdgeType::PROVIDEDBY:
        result["label"] = "provided by";
        result["color"] = "red";
        break;

      case CppEdgeType::CONTAINS:
        result["label"] = "contains";
        result["color"] = "blue";
        break;

      case CppEdgeType::CONTAINEDBY:
        result["label"] = "contained by";
        result["color"] = "blue";
        break;
        
      case CppEdgeType::SUBDIR:
        result["label"] = "subdir";
        result["color"] = "green";
        break;
        
      case CppEdgeType::IMPLEMENTS:
        result["label"] = "implements";
        result["color"] = "red";
        break;
        
      case CppEdgeType::IMPLEMENTEDBY:
        result["label"] = "implemented by";
        result["color"] = "red";
        break;
        
      case CppEdgeType::DEPENDS:
        result["label"] = "depends on";
        result["color"] = "blue";
        break;
        
      case CppEdgeType::DEPENDENTON:
        result["label"] = "dependent on";
        result["color"] = "blue";
        break;
    }
    
    return result;
  }

  void doAdditionalEdgeDecoration()
  {
    for (const auto& edgePair : edges)
    {
      _transaction([&, this]{
        typedef odb::query<model::Edge> EdgeQuery;
        typedef odb::result<model::Edge> EdgeResult;
        typedef odb::query<model::EdgeAttribute> EdgeAttrQuery;
        typedef odb::result<model::EdgeAttribute> EdgeAttrResult;
        
        std::string relationType;
        model::Edge::Type edgeType;
        switch (edgePair.second.edgeType)
        {
          case CppEdgeType::PROVIDES: case CppEdgeType::PROVIDEDBY:
            relationType = "provide";
            edgeType = model::Edge::PROVIDE;
            break;
          case CppEdgeType::DEPENDS: case CppEdgeType::DEPENDENTON:
            relationType = "depend";
            edgeType = model::Edge::DEPEND;
            break;
          case CppEdgeType::IMPLEMENTS: case CppEdgeType::IMPLEMENTEDBY:
            relationType = "implement";
            edgeType = model::Edge::IMPLEMENT;
            break;
          default:
            graph.setAttribute(edgePair.second.edge, "labeltooltip", "");
            return;
        }
        
        EdgeResult edge = _db->query<model::Edge>(
          EdgeQuery::from->domainId == std::to_string(edgePair.first.first) &&
          EdgeQuery::to->domainId   == std::to_string(edgePair.first.second) &&
          EdgeQuery::type           == edgeType);
        
        if (edge.empty())
          return;
        
        EdgeAttrResult attrs = _db->query<model::EdgeAttribute>(
          EdgeAttrQuery::edge->id == edge.begin()->id &&
          EdgeAttrQuery::key      == relationType);
        
        std::string label;
        for (const model::EdgeAttribute& attr : attrs)
          label += attr.value + "\n";
        
        graph.setAttribute(edgePair.second.edge, "labeltooltip", label);
      });
    }
  }
  
private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  
  std::string getLastNParts(const std::string& path, std::size_t n)
  {
    if (path.empty() || n == 0)
      return "";

    std::size_t p;
    for (p = path.rfind('/');
         --n > 0 && p - 1 < path.size();
         p = path.rfind('/', p - 1));

    return p > 0 && p < path.size() ? "..." + path.substr(p) : path; 
  }

  model::File getFile(const model::FileId& fileId)
  {
    model::File file;
    _transaction([&, this]{ _db->load<model::File>(fileId, file); });
    return file;
  }
};

}
}
}

#endif
