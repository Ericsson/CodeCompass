#include <boost/filesystem.hpp>
#include <model/file.h>
#include <algorithm>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>



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
    std::string& _return,
    const core::FileId& fileId)
{
  util::Graph graph_;
  std::string colAttr = "border='0' align='left'";

  std::string label = "<p><strong> FileType: ";
  LOG(info) << "getYAMLINFO is being Called" <<std::endl; 
  _transaction([&, this](){
    typedef odb::query<model::Yaml> YamlQuery;
    typedef odb::result<model::Yaml> YamlResult;
    typedef odb::result<model::YamlContent> YamlContentResult;
    typedef odb::query<model::YamlContent> YamlContentQuery;

    typedef odb::result<model::File> FilePathResult;
    typedef odb::query<model::File> FilePathQuery;
    
    FilePathResult yamlPath = _db->query<model::File>(
          FilePathQuery::id == std::stoull(fileId));
    YamlResult yamlInfo = _db->query<model::Yaml>(
          YamlQuery::file == std::stoull(fileId));

    core::FileInfo fileInfo;
    _projectService.getFileInfo(fileInfo, fileId);
    util::Graph::Node node = addNode(graph_, fileInfo);

    YamlContentResult yamlContent = _db->query<model::YamlContent>(
        YamlContentQuery::file == std::stoull(fileId));
    std::string list[] = {"KUBERNETES_CONFIG", "DOCKERFILE", "HELM_CHART", "CI", "OTHER"};
    std::stringstream ssId;
    std::stringstream ssType;
    int numOfDataPairs = yamlContent.size();
    ssId << yamlInfo.begin()->id;
    std::string type = list[yamlInfo.begin()->type];
    std::string path = yamlPath.begin()->path;
    label +=  type + "</p>";
    label += "<p><strong> FilePath: " + path + "</strong></p>";
    label += "<p><strong> Num of key-data pairs in the file: " + std::to_string(numOfDataPairs) + "</strong></p>";
    label += graphHtmlTag("p",
          graphHtmlTag("strong", "The main keys from the file are:") );
    label += "<ul>";

    for (const model::YamlContent& yc : yamlContent)
    {
      if (yc.parent == "")
      {
        label += graphHtmlTag("li", yc.key);
      }
    }
    label += "</ul>";
    graph_.setNodeAttribute(node, "FileInfo", label, true);
  });
  _return = label;
}


void YamlServiceHandler::getYamlFileDiagram(
    std::string& _return,
    const core::FileId& fileId)
{
    util::Graph graph_;
    std::string colAttr = "border='0' align='left'";
    std::string label = "<table border='1' cellspacing='0'>";
    _transaction([&, this](){
        typedef odb::result<model::YamlContent> YamlResult;
        typedef odb::query<model::YamlContent> YamlQuery;

        YamlResult yamlContent = _db->query<model::YamlContent>(
            YamlQuery::file == std::stoull(fileId));
        core::FileInfo fileInfo;
        _projectService.getFileInfo(fileInfo, fileId);
        util::Graph::Node node = addNode(graph_, fileInfo);
        
        label += graphHtmlTag("tr",
              graphHtmlTag("td", "Key", colAttr) + 
              graphHtmlTag("td", "Parent", colAttr) +
              graphHtmlTag("td", "Data", colAttr));
        
        for (const model::YamlContent& yc : yamlContent)
        {
            label += graphHtmlTag("tr",
                graphHtmlTag("td", yc.key, colAttr) + 
                graphHtmlTag("td", yc.parent, colAttr) +
                graphHtmlTag("td", yc.data, colAttr));
        }
        label.append("</table>");
        
        graph_.setNodeAttribute(node, "content", label, true);

    });
    _return = label;
}

util::Graph::Node YamlServiceHandler::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  util::Graph::Node node = graph_.getOrCreateNode(fileInfo_.id);
  graph_.setNodeAttribute(node, "label", getLastNParts(fileInfo_.path, 3));

  if (fileInfo_.type == model::File::DIRECTORY_TYPE)
  {
    decorateNode(graph_, node, directoryNodeDecoration);
  }
  else if (fileInfo_.type == model::File::BINARY_TYPE)
  {
    decorateNode(graph_, node, binaryFileNodeDecoration);
  }
  else
  {
    std::string ext = boost::filesystem::extension(fileInfo_.path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".yaml" || ext == ".yml")
      decorateNode(graph_, node, sourceFileNodeDecoration);
  }

  return node;
}

std::string YamlServiceHandler::getLastNParts(const std::string& path_, std::size_t n_)
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



const YamlServiceHandler::Decoration YamlServiceHandler::sourceFileNodeDecoration = {
  {"shape", "box"},
  {"style", "filled"},
  {"fillcolor", "#116db6"},
  {"fontcolor", "white"}
};


const YamlServiceHandler::Decoration YamlServiceHandler::directoryNodeDecoration = {
  {"shape", "folder"}
};

const YamlServiceHandler::Decoration YamlServiceHandler::binaryFileNodeDecoration = {
  {"shape", "box3d"},
  {"style", "filled"},
  {"fillcolor", "#f18a21"},
  {"fontcolor", "white"}
};

}
}
}