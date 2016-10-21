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
    const std::vector<std::pair<std::string, std::string>>& attrs_);

  void addEdge(
    const std::string& label_,
    const std::vector<std::pair<std::string, std::string>>& attrs_);

  Graph::Subgraph addSubgraph(const std::string& label_, Graph::Node& hook_);

  void setStyle(
    const Graph::Node& node_,
    const std::vector<std::pair<std::string, std::string>>& attrs_);

  void setStyle(
    const Graph::Edge& edge_,
    const std::vector<std::pair<std::string, std::string>>& attrs_);

private:
  Graph::Node registerSubgraph(const Graph::Subgraph& sub_);

  Graph _graph;
  Graph::Node _root;
};

}
}

#endif
