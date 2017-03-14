#ifndef CC_UTIL_LEGENDBUILDER_H
#define CC_UTIL_LEGENDBUILDER_H

#include <vector>
#include <string>
#include "graph.h"

namespace cc
{
namespace util
{

class LegendBuilder
{
public:
  LegendBuilder(const std::string& title_ = "");

  std::string getOutput() const;

  void addNode(
    const std::string& label_,
    const std::vector<std::pair<std::string, std::string>>& attrs_,
    bool html_ = false);

  void addEdge(
    const std::string& label_,
    const std::vector<std::pair<std::string, std::string>>& attrs_,
    bool html_ = false);

  Graph::Subgraph addSubgraph(const std::string& label_, Graph::Node& hook_);

  void setNodeStyle(
    const Graph::Node& node_,
    const std::vector<std::pair<std::string, std::string>>& attrs_,
    bool html_ = false);

  void setEdgeStyle(
    const Graph::Edge& edge_,
    const std::vector<std::pair<std::string, std::string>>& attrs_,
    bool html_ = false);

private:
  Graph::Node registerSubgraph(const Graph::Subgraph& sub_);

  Graph _graph;
  Graph::Node _root;
};

}
}

#endif
