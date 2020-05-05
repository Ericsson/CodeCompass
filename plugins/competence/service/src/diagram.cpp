#include "diagram.h"

#include <cmath>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/useremail.h>
#include <model/useremail-odb.hxx>

#include <util/legendbuilder.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace service
{
namespace competence
{

typedef odb::query<model::FileComprehension> FileComprehensionQuery;
typedef odb::query<model::File> FileQuery;

CompetenceDiagram::CompetenceDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _compHandler(db_, datadir_, context_),
      _projectHandler(db_, datadir_, context_),
      _transaction(db_)
{
}

void CompetenceDiagram::getCompetenceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  if (!fileInfo.isDirectory)
  {
    _transaction([&, this]()
    {
     auto comp = _db->query<model::FileComprehension>(
       FileComprehensionQuery::file == std::stoull(fileInfo.id));

     std::string color = "#ffffff";

     if (!comp.empty())
       color = rateToColor(comp.begin()->userRatio);

     Decoration competenceNodeDecoration = {
       {"shape", "box"},
       {"style", "filled"},
       {"fillcolor", color}
     };
     decorateNode(graph_, currentNode, competenceNodeDecoration);
    });

    return;
  }

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const std::pair<util::Graph::Node, uint16_t> node : getFileCompetenceRates(graph_, subdir))
    {
      util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
      decorateEdge(graph_, edge, containsEdgeDecoration);

      std::string color = rateToColor(node.second);

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }

  if (graph_.nodeCount() > nodeCountBorder)
  {
    graph_.setAttribute("rankdir", "LR");
  }
}

std::map<util::Graph::Node, uint16_t> CompetenceDiagram::getFileCompetenceRates(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::map<core::FileId, uint16_t> comprehension;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
      FileQuery::parent == std::stoull(node_) &&
      FileQuery::type != model::File::DIRECTORY_TYPE
    );

    for (const model::File& file : contained)
    {
      if (file.path.find(".git") != std::string::npos ||
          file.path.find(".idea") != std::string::npos)
        continue;

      auto comp = _db->query<model::FileComprehension>(
        FileComprehensionQuery::file == file.id
        );

      for (const model::FileComprehension& c : comp)
      {
        comprehension.insert(std::make_pair(std::to_string(c.file), c.userRatio));
      }
    }
  });

  std::map<util::Graph::Node, uint16_t> annotated;

  for (const auto& pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    annotated.insert(std::make_pair(addNode(graph_, fileInfo), pair.second));
  }

  return annotated;
}

util::Graph::Node CompetenceDiagram::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  util::Graph::Node node = graph_.getOrCreateNode(fileInfo_.id);
  graph_.setNodeAttribute(node, "label", fileInfo_.name);

  return node;
}

util::Graph::Node CompetenceDiagram::addNode(
  util::Graph& graph_,
  const core::FileId& fileId_,
  DiagramType type_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node node = graph_.getOrCreateNode(fileInfo.id);
  graph_.setNodeAttribute(node, "label", fileInfo.name);

  switch (type_)
  {
    case USER:
    case TEAM:
      std::string color = getExpertOnFile(fileId_);

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };

      decorateNode(graph_, node, competenceNodeDecoration);
      break;
  }

  return node;
}

std::string CompetenceDiagram::getExpertOnFile(
  const core::FileId& fileId_)
{
  _transaction([&, this]
  {
    odb::result<model::FileComprehension> comprehensionData = _db->query<model::FileComprehension>(
      FileComprehensionQuery::file == std::stoull(fileId_)
    );

    if (!comprehensionData.empty())
    {
      auto expert = *(comprehensionData.begin());
      for (auto it = comprehensionData.begin(); it != comprehensionData.end(); ++it)
        if (it->userRatio > expert.userRatio)
          expert = *it;

      auto userData = _db->query_one<model::UserEmail>(
        odb::query<model::UserEmail>::email == expert.userEmail
      );

      if (userData)
        return userData->colorCode;
    }
  });

  return "#ffffff";
}

std::string CompetenceDiagram::rateToColor(uint16_t rate)
{
  int red, green;

  red = rate < 50 ? 255 : std::round(256 - (rate - 50) * 5.12);
  green = rate > 50 ? 255 : std::round(rate * 5.12);

  if (green == 256)
    --green;

  if (red == 256)
    --red;

  std::stringstream ss;
  ss << "#";
  if (red != 0)
  {
    ss << std::hex << (red << 16 | green << 8 | 0);
  }
  else
  {
    ss << std::hex << 0 << 0 << (green << 8 | 0);
  }

  return ss.str();
}

std::vector<util::Graph::Node> CompetenceDiagram::getSubDirs(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> subdirs;

  _transaction([&, this]
   {
     odb::result<model::File> sub = _db->query<model::File>(
       FileQuery::parent == std::stoull(node_) &&
       FileQuery::type == model::File::DIRECTORY_TYPE);

     for (const model::File& subdir : sub)
     {
       if (subdir.path.find(".git") != std::string::npos ||
           subdir.path.find(".idea") != std::string::npos)
         continue;

       core::FileInfo fileInfo;
       _projectHandler.getFileInfo(fileInfo, std::to_string(subdir.id));

       util::Graph::Node node = addNode(graph_, fileInfo);
       decorateNode(graph_, node, directoryNodeDecoration);
       subdirs.push_back(node);
     }
   });

  return subdirs;
}

void CompetenceDiagram::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void CompetenceDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

const CompetenceDiagram::Decoration CompetenceDiagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::directoryNodeDecoration = {
  {"shape", "folder"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::containsEdgeDecoration = {
  {"label", "contains"},
  {"color", "blue"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::subdirEdgeDecoration = {
  {"label", "subdir"},
  {"color", "green"}
};
}
}
}