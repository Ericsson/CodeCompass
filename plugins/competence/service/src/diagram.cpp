#include "diagram.h"

#include <cmath>
#include <cctype>

#include <util/legendbuilder.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace service
{
namespace competence
{

std::map<char, std::uint32_t> CompetenceDiagram::_charCodes;
std::map<std::string, std::string> CompetenceDiagram::_userColorCodes;
std::map<std::string, std::string> CompetenceDiagram::_companyColorCodes;

CompetenceDiagram::CompetenceDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _compHandler(db_, datadir_, context_),
      _projectHandler(db_, datadir_, context_),
      _transaction(db_)
{
  srand(time(nullptr));
  setCharCodes();
  setUserColorCodes();
  setCompanyColorCodes();
}

void CompetenceDiagram::getCompetenceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_,
  std::string user_,
  const std::int32_t diagramType_)
{
  switch (diagramType_)
  {
    case 0:
      userViewDiagram(graph_, fileId_, user_);
      break;
    case 1:
      teamViewDiagram(graph_, fileId_);
      break;
    case 2:
      personalCompanyViewDiagram(graph_, fileId_);
      break;
    case 3:
      accumulatedCompanyViewDiagram(graph_, fileId_);
      break;
  }
}

void CompetenceDiagram::userViewDiagram(
  util::Graph &graph_,
  const core::FileId &fileId_,
  std::string &user_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  std::vector<std::string> emails;
  _transaction([&, this]()
  {
    auto emailQuery = _db->query<model::UserEmail>(
      UserEmailQuery::username == user_);

    for (const auto& e : emailQuery)
      emails.push_back(e.email);
  });

  if (user_ == "Anonymous" || emails.empty())
  {
    Decoration competenceNodeDecoration = {
      {"shape", "box"},
      {"style", "filled"},
      {"fillcolor", _white}
    };
    decorateNode(graph_, currentNode, competenceNodeDecoration);
    return;
  }

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const auto& node : getFileCompetenceRates(graph_, subdir, emails))
    {
      if (node.first != subdir)
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
        decorateEdge(graph_, edge, containsEdgeDecoration);
      }

      std::string color = rateToColor(node.second);

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }

  if (graph_.nodeCount() > _nodeCountLimit)
    graph_.setAttribute("rankdir", "LR");
}

/*
 * An individual user's competence rate is the maximum percentage
 * that is persisted along with any of the user's email addresses.
 */
std::map<util::Graph::Node, int16_t> CompetenceDiagram::getFileCompetenceRates(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const std::vector<std::string>& emails_)
{
  std::map<core::FileId, int16_t> comprehension;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
      FileQuery::parent == std::stoull(node_) &&
      FileQuery::type != model::File::DIRECTORY_TYPE ||
      FileQuery::id == std::stoull(node_)
    );

    for (const model::File& file : contained)
    {
      auto fileComp = _db->query<model::FileComprehension>(
        FileComprehensionQuery::file == file.id);

      std::vector<model::FileComprehension> comp;
      for (const model::FileComprehension& f : fileComp)
        for (const std::string& e : emails_)
          if (f.userEmail == e)
          {
            comp.push_back(f);
            break;
          }

      if (!comp.empty())
      {
        auto max = std::max_element(comp.begin(), comp.end(),
          [](const model::FileComprehension& a, const model::FileComprehension& b)
          {
            return a.userRatio < b.userRatio;
          });

        comprehension.insert(std::make_pair(std::to_string(max->file), max->userRatio));
      }
      else
      {
        comprehension.insert(std::make_pair(std::to_string(file.id), -1));
      }
    }
  });

  std::map<util::Graph::Node, int16_t> annotated;
  for (const auto& pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    annotated.insert(std::make_pair(addNode(graph_, fileInfo), pair.second));
  }

  return annotated;
}

std::string CompetenceDiagram::getUserViewDiagramLegend()
{
  util::LegendBuilder builder("User Competence Diagram");

  //builder.addNode()

  return builder.getOutput();
}

void CompetenceDiagram::teamViewDiagram(
  util::Graph &graph_,
  const core::FileId &fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const auto& node : getFileExpertNodes(graph_, subdir))
    {
      if (node.first != subdir)
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
        decorateEdge(graph_, edge, containsEdgeDecoration);
      }

      std::string color = generateColor(node.second);

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }

  if (graph_.nodeCount() > _nodeCountLimit)
    graph_.setAttribute("rankdir", "LR");
}

/*
 * The expert user of a file is the one with the
 * maximum competence percentage.
 */
std::map<util::Graph::Node, std::string> CompetenceDiagram::getFileExpertNodes(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::map<core::FileId, std::string> comprehension;
  _transaction([&, this]
  {
   auto contained = _db->query<model::File>(
     FileQuery::parent == std::stoull(node_) &&
     FileQuery::type != model::File::DIRECTORY_TYPE ||
     FileQuery::id == std::stoull(node_)
   );

   for (const model::File& file : contained)
   {
     auto comp = _db->query<model::FileComprehension>(
       FileComprehensionQuery::file == file.id);

     if (!comp.empty())
     {
       auto max = maxCompetence(comp);
       comprehension.insert(std::make_pair(std::to_string(max.file), max.userEmail));
     }
     else
     {
       comprehension.insert(std::make_pair(std::to_string(file.id), ""));
     }
   }
  });

  std::map<util::Graph::Node, std::string> annotated;
  for (const auto& pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    annotated.insert(std::make_pair(addNode(graph_, fileInfo), pair.second));
  }

  return annotated;
}

std::string CompetenceDiagram::getTeamViewDiagramLegend()
{
  util::LegendBuilder builder("Team Competence Diagram");

  for (const auto& code : _userColorCodes)
    builder.addNode(code.first, {{"shape", "box"},
      {"style", "filled"}, {"fillcolor", code.second}});

  builder.addNode("No data", {{"shape", "box"},
    {"style", "filled"}, {"fillcolor", _white}});

  return builder.getOutput();
}

void CompetenceDiagram::personalCompanyViewDiagram(
  util::Graph &graph_,
  const core::FileId &fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const auto& node : getCompanyExpertNodes(graph_, subdir))
    {
      if (node.first != subdir)
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
        decorateEdge(graph_, edge, containsEdgeDecoration);
      }

      std::string color = _white;
      if (_companyColorCodes.find(node.second) != _companyColorCodes.end())
        color = _companyColorCodes[node.second];

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }

  if (graph_.nodeCount() > _nodeCountLimit)
    graph_.setAttribute("rankdir", "LR");
}

std::map<util::Graph::Node, std::string> CompetenceDiagram::getCompanyExpertNodes(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::map<core::FileId, std::string> comprehension;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
     FileQuery::parent == std::stoull(node_) &&
     FileQuery::type != model::File::DIRECTORY_TYPE ||
     FileQuery::id == std::stoull(node_)
   );

    for (const model::File& file : contained)
    {
      auto comp = _db->query<model::FileComprehension>(
        FileComprehensionQuery::file == file.id);

      if (!comp.empty())
      {
        auto max = maxCompetence(comp);
        auto maxCompany = getCompany(max.userEmail);
        comprehension.insert(std::make_pair(std::to_string(max.file), maxCompany));
      }
      else
      {
        comprehension.insert(std::make_pair(std::to_string(file.id), ""));
      }
    }
  });

  std::map<util::Graph::Node, std::string> annotated;
  for (const auto& pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    annotated.insert(std::make_pair(addNode(graph_, fileInfo), pair.second));
  }

  return annotated;
}

void CompetenceDiagram::accumulatedCompanyViewDiagram(
  util::Graph &graph_,
  const core::FileId &fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const auto& node : getAccumulatedCompanyExpertNodes(graph_, subdir))
    {
      if (node.first != subdir)
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
        decorateEdge(graph_, edge, containsEdgeDecoration);
      }

      std::string color = _white;
      if (_companyColorCodes.find(node.second) != _companyColorCodes.end())
        color = _companyColorCodes[node.second];

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }

  if (graph_.nodeCount() > _nodeCountLimit)
    graph_.setAttribute("rankdir", "LR");
}

std::map<util::Graph::Node, std::string> CompetenceDiagram::getAccumulatedCompanyExpertNodes(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::map<core::FileId, std::string> comprehension;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
      FileQuery::parent == std::stoull(node_) &&
      FileQuery::type != model::File::DIRECTORY_TYPE ||
      FileQuery::id == std::stoull(node_)
    );

   for (const model::File &file : contained)
   {
     auto comp = _db->query<model::FileComprehension>(
       FileComprehensionQuery::file == file.id);

     if (!comp.empty())
     {
       auto maxCompany = maxCompanyCompetence(comp);
       comprehension.insert(std::make_pair(std::to_string(file.id), maxCompany));
     }
     else
     {
       comprehension.insert(std::make_pair(std::to_string(file.id), ""));
     }
   }
  });

  std::map<util::Graph::Node, std::string> annotated;
  for (const auto &pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    annotated.insert(std::make_pair(addNode(graph_, fileInfo), pair.second));
  }

  return annotated;
}

/* *
 * IMPORTANT: Calculation method is under development.
 * For now, it only returns the company with the highest
 * average competence ratio.
 */
std::string CompetenceDiagram::maxCompanyCompetence(
  FileComprehensionResult& result)
{
  std::map<std::string, int> companyRatios;
  for (const auto& res : result)
  {
    std::string currCompany = getCompany(res.userEmail);
    if (companyRatios.find(currCompany) == companyRatios.end())
    {
      auto ratio = 0, counter = 0;
      for (const auto& r : result)
        if (getCompany(r.userEmail) == currCompany)
        {
          ratio += r.userRatio;
          ++counter;
        }

      ratio /= counter;
      companyRatios.insert({currCompany, ratio});
    }
  }

  auto max = std::max_element(companyRatios.begin(), companyRatios.end(),
    [&](const auto& a, const auto& b){
      return a.second < b.second;
  });

  return max->first;
}

std::string CompetenceDiagram::getCompany(const std::string& email_)
{
  std::string company;

  _transaction([&, this]()
  {
    auto user = _db->query_one<model::UserEmail>(
      UserEmailQuery::email == email_);

    if (!user)
    {
      company = "";
      return;
    }
    company = user.get()->company;
  });

  return company;
}

/*
 * Note: The ODB manual description about the odb::result::iterator type:
 * "If we have two iterators pointing to the current position
 * and then we advance one of them, the other will advance as well."
 * Thus, std::max_element cannot be used for maximum search.
 */
model::FileComprehension CompetenceDiagram::maxCompetence(
  FileComprehensionResult& result)
{
  model::FileComprehension max = *(result.begin());
  for (const auto &f : result)
    if (f.userRatio > max.userRatio)
      max = f;

  return max;
}

util::Graph::Node CompetenceDiagram::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  util::Graph::Node node = graph_.getOrCreateNode(fileInfo_.id);
  graph_.setNodeAttribute(node, "label", fileInfo_.name);

  return node;
}

std::string CompetenceDiagram::rateToColor(int16_t rate)
{
  if (rate < 0 || rate > 100)
    return _white;

  int red, green;
  red = rate < 50 ? 255 : (int)std::round(256 - (rate - 50) * 5.12);
  green = rate > 50 ? 255 : (int)std::round(rate * 5.12);

  if (green == 256)
    --green;

  if (red == 256)
    --red;

  std::stringstream ss;
  ss << "#";
  if (red != 0)
    ss << std::hex << (red << 16 | green << 8 | 0);
  else
    ss << std::hex << 0 << 0 << (green << 8 | 0);

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

std::string CompetenceDiagram::generateColor(const std::string& email_)
{
  if (_userColorCodes.find(email_) != _userColorCodes.end())
    return _userColorCodes.at(email_);

  if (email_.size() < 6)
    return _white;

  std::stringstream ss;
  ss << "#";

  int red, green, blue;
  red = (_charCodes[email_.at(0)] + _charCodes[email_.at(1)]) * 3;
  green = (_charCodes[email_.at(2)] + _charCodes[email_.at(3)]) * 3;
  blue = (_charCodes[email_.at(4)] + _charCodes[email_.at(5)]) * 3;

  ss << std::hex << (red << 16 | green << 8 | blue);

  _userColorCodes.insert(std::make_pair(email_, ss.str()));

  return ss.str();
}

void CompetenceDiagram::setCharCodes()
{
  if (!_charCodes.empty())
    return;

  int counter = 10;
  for (int i = 0; i < counter; ++i)
    _charCodes.insert({'0' + i, i});

  for (char c = 'a'; c <= 'z'; ++c)
  {
    _charCodes.insert({c, counter});
    ++counter;
  }

  for (char c = 'A'; c <= 'Z'; ++c)
    _charCodes.insert({c, _charCodes[tolower(c)]});

  std::vector<char> misc = { ',', '@', '!', '#', '$', '%', '&', '\'',
                             '*', '+', '-', '/', '=', '?', '^', '_',
                             '`', '{', '|', '}', '~' };

  for (const char& c : misc)
  {
    _charCodes.insert({c, counter});
    ++counter;
  }

  _charCodes.insert({' ', counter});
}

void CompetenceDiagram::setUserColorCodes()
{
  if (!_userColorCodes.empty())
    return;

  _transaction([&, this]
  {
    auto emails = _db->query<model::UserEmail>();

    for (const model::UserEmail& user : emails)
      generateColor(user.email);
  });
}


void CompetenceDiagram::setCompanyColorCodes()
{
  if (!_companyColorCodes.empty())
    return;

  _transaction([&, this]
  {
   auto emails = _db->query<model::UserEmail>();

   for (const model::UserEmail& user : emails)
     if (_companyColorCodes.find(user.company) == _companyColorCodes.end())
     {
       int red, green, blue;
       red = rand() % 256;
       green = rand() % 256;
       blue = rand() % 256;
       std::stringstream ss;
       ss << "#" << std::hex << (red << 16 | green << 8 | blue);
       _companyColorCodes.insert({user.company, ss.str()});
     }
  });
}

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