#include <algorithm>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

#include <model/file.h>
#include <model/yamlastnode.h>
#include <yamlservice/yamlservice.h>

namespace
{
  typedef odb::query<cc::model::YamlAstNode> AstQuery;
  typedef odb::result<cc::model::YamlAstNode> AstResult;
}

namespace cc
{
namespace service
{
namespace language
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

void YamlServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("YAML");
}

void YamlServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{
  //return_ = _transaction([this, &astNodeId_](){
  //    return CreateAstNodeInfo()(queryCppAstNode(astNodeId_));
}

void YamlServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_)
{
}

void YamlServiceHandler::getSourceText(
        std::string& return_,
        const core::AstNodeId& astNodeId_)
{  }

void YamlServiceHandler::getDocumentation(
        std::string& return_,
        const core::AstNodeId& astNodeId_) {}

void YamlServiceHandler::getProperties(
        std::map<std::string, std::string>& return_,
        const core::AstNodeId& astNodeId_) {}

void YamlServiceHandler::getDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_) {}

void YamlServiceHandler::getDiagram(
        std::string& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t diagramId_) {}

void YamlServiceHandler::getDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_) {}

void YamlServiceHandler::getFileDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_) {}

void YamlServiceHandler::getFileDiagram(
        std::string& return_,
        const core::FileId& fileId_,
        const int32_t diagramId_) {}

void YamlServiceHandler::getFileDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_) {}

void YamlServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId)
{
  /*model::YamlAstNode node = queryYamlAstNode(astNodeId_);

  switch (node.symbolType)
  {
    case
  }*/
}

void YamlServiceHandler::getReferences(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::vector<std::string>& tags_) {}

std::int32_t YamlServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_)
{
  model::YamlAstNode node = queryYamlAstNode(astNodeId_);

  /*return _transaction([&, this]() -> std::int32_t {
    switch (referenceId_)
    {
      case
    }
  });*/
}

void YamlServiceHandler::getReferencesInFile(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const core::FileId& fileId_,
        const std::vector<std::string>& tags_) {}

void YamlServiceHandler::getReferencesPage(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::int32_t pageSize_,
        const std::int32_t pageNo_) {}

void YamlServiceHandler::getFileReferenceTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_) {}

void YamlServiceHandler::getFileReferences(
        std::vector<AstNodeInfo>& return_,
        const core::FileId& fileId_,
        const std::int32_t referenceId_) {}

std::int32_t YamlServiceHandler::getFileReferenceCount(
        const core::FileId& fileId_,
        const std::int32_t referenceId_) {}

void YamlServiceHandler::getSyntaxHighlight(
        std::vector<SyntaxHighlight>& return_,
        const core::FileRange& range_) {}

model::YamlAstNode YamlServiceHandler::queryYamlAstNode(
  const core::AstNodeId &astNodeId_)
{
  return _transaction([&, this](){
    model::YamlAstNode node;

    if (!_db->find(std::stoull(astNodeId_), node))
    {
      core::InvalidId ex;
      ex.__set_msg("Invalid YamlAstNode ID");
      ex.__set_nodeid(astNodeId_);
      throw ex;
    }

    return node;
  });
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
