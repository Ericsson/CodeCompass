#include <algorithm>
#include <regex>
#include <sstream>

#include <boost/filesystem.hpp>

#include <model/file.h>
#include <service/yamlservice.h>
#include <util/dbutil.h>

#include "yamlfilediagram.h"

namespace cc
{
namespace service
{
namespace language
{

namespace fs = boost::filesystem;

YamlFileDiagram::YamlFileDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _yamlHandler(db_, datadir_, context_),
    _projectHandler(db_, datadir_, context_)
{
}

void YamlFileDiagram::getYamlFileInfo(
        util::Graph& graph_,
        const core::FileId& fileId_)
{
  std::string htmlContent = "";
    _transaction([&, this](){

      FilePathResult yamlPath = _db->query<model::File>(
    FilePathQuery::id == std::stoull(fileId_));

      YamlResult yamlInfo = _db->query<model::YamlFile>(
    YamlQuery::file == std::stoull(fileId_));

      YamlContentResult yamlContent = _db->query<model::YamlContent>(
    YamlContentQuery::file == std::stoull(fileId_));

      core::FileInfo fileInfo;
      _projectHandler.getFileInfo(fileInfo, fileId_);
      util::Graph::Node node = addNode(graph_, fileInfo);

      int numOfDataPairs = yamlContent.size();
      std::string type = typeToString(yamlInfo.begin()->type);
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
        htmlContent += graphHtmlTag("li", yc.key);
      }

      htmlContent += "</ul>";
      graph_.setNodeAttribute(node, "FileInfo", htmlContent, true);
  });
  //return_ = htmlContent;
}


void YamlFileDiagram::getYamlFileDiagram(
        util::Graph& graph_,
        const core::FileId& fileId_)
{
  //util::Graph graph_;
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
      _projectHandler.getFileInfo(fileInfo, fileId_);
      util::Graph::Node node = addNode(graph_, fileInfo);

      table += graphHtmlTag("tr",
        graphHtmlTag("th", "Key", thAttr) +
        //graphHtmlTag("th", "Parent", thAttr) +
        graphHtmlTag("th", "Value", thAttr));

      for (const model::YamlContent& yc : yamlContent)
      {
        table += graphHtmlTag("tr",
          graphHtmlTag("td", yc.key, tdAttr) +
          //graphHtmlTag("td", yc.parent, tdAttr) +
          graphHtmlTag("td", yc.value, tdAttr));
      }
      table.append("</table>");

      graph_.setNodeAttribute(node, "content", table, true);
  });
  //return_ = table;
}

void YamlFileDiagram::getMicroserviceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getMicroservices,
    this, std::placeholders::_1, std::placeholders::_2),
    microserviceNodeDecoration, {}, 1);

  LOG(warning) << graph_.output(util::Graph::SVG);
}

std::vector<util::Graph::Node> YamlFileDiagram::getMicroservices(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> microservices;

  std::vector<core::FileId> dirIds = getMicroserviceDirIds(graph_, node_);

  for (const core::FileId& dirId : dirIds)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, dirId);

    microservices.push_back(addNode(graph_, fileInfo));
  }

  return microservices;
}

std::vector<core::FileId> YamlFileDiagram::getMicroserviceDirIds(
  util::Graph&,
  const util::Graph::Node& node_)
{
  std::vector<core::FileId> microservices;

  _transaction([&, this] {
    odb::result<model::YamlFile> res = _db->query<model::YamlFile>(
      odb::query<model::YamlFile>::type == model::YamlFile::HELM_CHART);

    for (const model::YamlFile& yamlFile : res)
    {
      auto fileRes = _db->query_one<model::File>(
        odb::query<model::File>::id == yamlFile.file);

      fs::path filePath(fileRes->path);
      fs::path dirPath = filePath.parent_path();

      auto dirRes = _db->query_one<model::File>(
        odb::query<model::File>::path == dirPath.string());

      core::FileId dirId = std::to_string(dirRes->id);
      microservices.push_back(dirId);
    }
  });

  return microservices;
}

std::string YamlFileDiagram::graphHtmlTag(
  const std::string& tag_,
  const std::string& content_,
  const std::string& attr_)
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

util::Graph::Node YamlFileDiagram::addNode(
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

std::string YamlFileDiagram::getLastNParts(
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

void YamlFileDiagram::decorateNode(
        util::Graph& graph_,
        const util::Graph::Node& node_,
        const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

const YamlFileDiagram::Decoration
  YamlFileDiagram::sourceFileNodeDecoration = {
  {"shape", "box"},
  {"style", "filled"},
  {"fillcolor", "#116db6"},
  {"fontcolor", "white"}
};

const YamlFileDiagram::Decoration
        YamlFileDiagram::directoryNodeDecoration = {
  {"shape", "folder"}
};

const YamlFileDiagram::Decoration
  YamlFileDiagram::binaryFileNodeDecoration = {
  {"shape", "box3d"},
  {"style", "filled"},
  {"fillcolor", "#f18a21"},
  {"fontcolor", "white"}
};

const YamlFileDiagram::Decoration YamlFileDiagram::microserviceNodeDecoration = {
  {"shape", "folder"},
  {"color", "blue"}
};

}
}
}