#include <algorithm>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

#include <model/file.h>
#include <yamlservice/yamlservice.h>

namespace cc
{
namespace service
{
namespace yaml
{

YamlServiceHandler::YamlServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _projectService(db_, datadir_, context_)
{
}

std::string graphHtmlTag(
  const std::string& tag_,
  const std::string& content_,
  const std::string& attr_ = "")
{
  return std::string("<")
    .append(tag_)
    .append(" ")
    .append(attr_)
    .append(">")
    .append(content_)
    .append("</")
    .append(tag_)
    .append(">");
}

void YamlServiceHandler::getYamlFileInfo(
  std::string& return_,
  const core::FileId& fileId_)
{
  util::Graph graph_;

  std::string htmlContent = "";
  _transaction([&, this](){
    typedef odb::result<model::File> FilePathResult;
    typedef odb::query<model::File> FilePathQuery;

    typedef odb::query<model::Yaml> YamlQuery;
    typedef odb::result<model::Yaml> YamlResult;

    typedef odb::result<model::YamlContent> YamlContentResult;
    typedef odb::query<model::YamlContent> YamlContentQuery;

    FilePathResult yamlPath = _db->query<model::File>(
          FilePathQuery::id == std::stoull(fileId_));

    YamlResult yamlInfo = _db->query<model::Yaml>(
          YamlQuery::file == std::stoull(fileId_));

    YamlContentResult yamlContent = _db->query<model::YamlContent>(
        YamlContentQuery::file == std::stoull(fileId_));

    core::FileInfo fileInfo;
    _projectService.getFileInfo(fileInfo, fileId_);
    util::Graph::Node node = addNode(graph_, fileInfo);

    std::string typeList[] = 
    {
      "KUBERNETES_CONFIG",
      "DOCKER_COMPOSE",
      "HELM_CHART",
      "CI",
      "OTHER"
    };

    int numOfDataPairs = yamlContent.size();
    std::string type = typeList[yamlInfo.begin()->type];
    std::string path = yamlPath.begin()->path;

    htmlContent +=  "<p><strong> FileType: </strong>" + type + "</p>";
    htmlContent += "<p><strong> FilePath: </strong>" + path + "</p>";
    htmlContent += "<p><strong> Key-data pairs: </strong>"
     + std::to_string(numOfDataPairs) + "</p>";

    htmlContent += graphHtmlTag("p",
          graphHtmlTag("strong", "Main Keys:"));

    htmlContent += "<ul>";

    for (const model::YamlContent& yc : yamlContent)
    {
      if (yc.parent == "")
      {
        htmlContent += graphHtmlTag("li", yc.key);
      }
    }

    htmlContent += "</ul>";
    graph_.setNodeAttribute(node, "FileInfo", htmlContent, true);
  });
  return_ = htmlContent;
}


void YamlServiceHandler::getYamlFileDiagram(
  std::string& return_,
  const core::FileId& fileId_)
{
  util::Graph graph_;
  std::string thAttr 
    = "style=\"background-color:lightGray; font-weight:bold; height:50px\"";

  std::string tdAttr = "style=\"height:30px\"";
  std::string table = "<table border='1' cellspacing='1' width='75%'>";
  _transaction([&, this](){
    typedef odb::result<model::YamlContent> YamlResult;
    typedef odb::query<model::YamlContent> YamlQuery;

    YamlResult yamlContent = _db->query<model::YamlContent>(
        YamlQuery::file == std::stoull(fileId_));

    core::FileInfo fileInfo;
    _projectService.getFileInfo(fileInfo, fileId_);
    util::Graph::Node node = addNode(graph_, fileInfo);

    table += graphHtmlTag("tr",
          graphHtmlTag("th", "Key", thAttr) +
          graphHtmlTag("th", "Parent", thAttr) +
          graphHtmlTag("th", "Data", thAttr));

    for (const model::YamlContent& yc : yamlContent)
    {
      table += graphHtmlTag("tr",
          graphHtmlTag("td", yc.key, tdAttr) + 
          graphHtmlTag("td", yc.parent, tdAttr) +
          graphHtmlTag("td", yc.data, tdAttr));
    }
    table.append("</table>");

    graph_.setNodeAttribute(node, "content", table, true);

  });
  return_ = table;
}

util::Graph::Node YamlServiceHandler::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  util::Graph::Node node_ = graph_.getOrCreateNode(fileInfo_.id);
  graph_.setNodeAttribute(node_, "label", getLastNParts(fileInfo_.path, 3));

  if (fileInfo_.type == model::File::DIRECTORY_TYPE)
  {
    decorateNode(graph_, node_, directoryNodeDecoration);
  }
  else if (fileInfo_.type == model::File::BINARY_TYPE)
  {
    decorateNode(graph_, node_, binaryFileNodeDecoration);
  }
  else
  {
    std::string ext = boost::filesystem::extension(fileInfo_.path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".yaml" || ext == ".yml")
      decorateNode(graph_, node_, sourceFileNodeDecoration);
  }

  return node_;
}

std::string YamlServiceHandler::getLastNParts(
  const std::string& path_,
  std::size_t n_)
{
  if (path_.empty() || n_ == 0)
    return "";

  std::size_t p;
  for (p = path_.rfind('/');
       --n_ > 0 && p - 1 < path_.size();
       p = path_.rfind('/', p - 1));

  return p > 0 && p < path_.size() ? "..." + path_.substr(p) : path_;
}

void YamlServiceHandler::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}



const YamlServiceHandler::Decoration 
  YamlServiceHandler::sourceFileNodeDecoration = {
  {"shape", "box"},
  {"style", "filled"},
  {"fillcolor", "#116db6"},
  {"fontcolor", "white"}
};


const YamlServiceHandler::Decoration 
  YamlServiceHandler::directoryNodeDecoration = {
  {"shape", "folder"}
};

const YamlServiceHandler::Decoration
  YamlServiceHandler::binaryFileNodeDecoration = {
  {"shape", "box3d"},
  {"style", "filled"},
  {"fillcolor", "#f18a21"},
  {"fontcolor", "white"}
};

}
}
}
