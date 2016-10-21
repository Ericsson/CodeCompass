#ifndef CC_UTIL_DIAGRAM_LEGENDBUILDER_H
#define CC_UTIL_DIAGRAM_LEGENDBUILDER_H

#include <util/graph/graph.h>

namespace cc
{
namespace util
{
namespace diagram
{

class LegendBuilder
{
  util::Graph       graph;
  util::Graph::Node root;
  util::Graph::Node prevReference;
  util::Graph::Node registerSubgraph(util::Graph::Subgraph & sub);

public:

  LegendBuilder();
  LegendBuilder(const std::string& title);
  std::string getOutput();
  void addNode(const std::string & label, const std::vector<std::pair<std::string, std::string>> & attrs);
  void addEdge(const std::string & label, const std::vector<std::pair<std::string, std::string>> & attrs);
  util::Graph::Subgraph addSubgraph(const std::string & label,util::Graph::Node* ref1);
  void setStyle(util::Graph::Node node, const std::vector<std::pair<std::string, std::string>> & attrs);
  void setStyle(util::Graph::Edge edge, const std::vector<std::pair<std::string, std::string>> & attrs);
  util::Graph* getGraph();
};

} // diagram
} // util
} // cc

#endif // CC_UTIL_DIAGRAM_LEGENDBUILDER_H
