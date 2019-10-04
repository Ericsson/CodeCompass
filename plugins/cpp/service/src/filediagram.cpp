#include <boost/filesystem.hpp>

#include <model/cppheaderinclusion.h>
#include <model/cppheaderinclusion-odb.hxx>

#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>

#include <model/cppedge.h>
#include <model/cppedge-odb.hxx>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/legendbuilder.h>

#include "filediagram.h"

namespace cc
{
namespace service
{
namespace language
{

typedef odb::query<model::CppHeaderInclusion> IncludeQuery;
typedef odb::result<model::CppHeaderInclusion> IncludeResult;
typedef odb::query<model::CppEdge> EdgeQuery;
typedef odb::result<model::CppEdge> EdgeResult;
typedef odb::query<model::BuildTarget> TargetQuery;
typedef odb::result<model::BuildTarget> TargetResult;
typedef odb::query<model::BuildSource> SourceQuery;
typedef odb::result<model::BuildSource> SourceResult;
typedef odb::query<model::File> FileQuery;
typedef odb::result<model::File> FileResult;

FileDiagram::FileDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _cppHandler(db_, datadir_, context_),
      _projectHandler(db_, datadir_, context_)
{
}

void FileDiagram::getComponentUsersDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  std::set<util::Graph::Node> provides = util::bfsBuild(graph_, currentNode,
    std::bind(&FileDiagram::getProvides, this, std::placeholders::_1,
    std::placeholders::_2), {}, providesEdgeDecoration, 1);

  std::set<util::Graph::Node> usedHeaders = provides;
  for (const util::Graph::Node& provide : provides)
  {
    std::set<util::Graph::Node> revusages = util::bfsBuild(graph_, provide,
      std::bind(&FileDiagram::getRevUsages, this, std::placeholders::_1,
      std::placeholders::_2), {}, revUsagesEdgeDecoration);

    for (const util::Graph::Node& revusage : revusages)
      usedHeaders.insert(revusage);
  }

  for (const util::Graph::Node& source : usedHeaders)
    util::bfsBuild(graph_, source, std::bind(&FileDiagram::getRevContains,
      this, std::placeholders::_1, std::placeholders::_2),
      {}, revContainsEdgeDecoration);
}

std::string FileDiagram::getComponentUsersDiagramLegend()
{
  util::LegendBuilder builder("Component Users Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("provides", providesEdgeDecoration);
  builder.addEdge("used by files", revUsagesEdgeDecoration);
  builder.addEdge("contained by", revContainsEdgeDecoration);

  return builder.getOutput();
}

void FileDiagram::getIncludeDependencyDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  util::bfsBuild(graph_, currentNode,std::bind(&FileDiagram::getUsages,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, usagesEdgeDecoration, 3);

  util::bfsBuild(graph_, currentNode,std::bind(&FileDiagram::getRevUsages,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, revUsagesEdgeDecoration, 3);
  
  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getProvides,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, usagesEdgeDecoration, 3);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getRevProvides,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, revUsagesEdgeDecoration, 3);
}

std::string FileDiagram::getIncludeDependencyDiagramLegend()
{
  util::LegendBuilder builder("Include Dependency Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("uses", usagesEdgeDecoration);
  builder.addEdge("used by files", revUsagesEdgeDecoration);

  return builder.getOutput();
}

void FileDiagram::getExternalDependencyDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&FileDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const util::Graph::Node& impl : getImplements(graph_, subdir))
      if (subdirs.find(impl) == subdirs.end())
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, impl);
        decorateEdge(graph_, edge, implementsEdgeDecoration);
      }

    for (const util::Graph::Node& dep : getDepends(graph_, subdir))
      if (subdirs.find(dep) == subdirs.end())
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, dep);
        decorateEdge(graph_, edge, dependsEdgeDecoration);
      }
  }
}

std::string FileDiagram::getExternalDependencyDiagramLegend()
{
  util::LegendBuilder builder("External Dependency Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("sub directory", subdirEdgeDecoration);
  builder.addEdge("implements", implementsEdgeDecoration);
  builder.addEdge("depends on", dependsEdgeDecoration);

  return builder.getOutput();
}

void FileDiagram::getExternalUsersDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&FileDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  for (const util::Graph::Node& subdir : subdirs)
  {
    util::bfsBuild(graph_, subdir,std::bind(&FileDiagram::getRevImplements,
      this, std::placeholders::_1, std::placeholders::_2),
      {}, revImplementsEdgeDecoration);

    util::bfsBuild(graph_, subdir,std::bind(&FileDiagram::getRevDepends,
      this, std::placeholders::_1, std::placeholders::_2),
      {}, revDependsEdgeDecoration);
  }
}

std::string FileDiagram::getExternalUsersDiagramLegend()
{
  util::LegendBuilder builder("External Users Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("sub directory", subdirEdgeDecoration);
  builder.addEdge("implemented by", revImplementsEdgeDecoration);
  builder.addEdge("dependent on", revDependsEdgeDecoration);

  return builder.getOutput();
}

void FileDiagram::getInterfaceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getProvides,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, providesEdgeDecoration, 1);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getContains,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, containsEdgeDecoration, 1);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getUsages,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, usagesEdgeDecoration, 1);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getRevProvides,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, revProvidesEdgeDecoration, 1);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getRevContains,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, revContainsEdgeDecoration, 1);

  util::bfsBuild(graph_, currentNode, std::bind(&FileDiagram::getRevUsages,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, revUsagesEdgeDecoration, 1);
}

std::string FileDiagram::getInterfaceDiagramLegend()
{
  util::LegendBuilder builder("Interface Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("provides", providesEdgeDecoration);
  builder.addEdge("contains", containsEdgeDecoration);
  builder.addEdge("uses", usagesEdgeDecoration);
  builder.addEdge("provided by", revProvidesEdgeDecoration);
  builder.addEdge("contained by", revContainsEdgeDecoration);
  builder.addEdge("used by", revUsagesEdgeDecoration);

  return builder.getOutput();
}

void FileDiagram::getSubsystemDependencyDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&FileDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const util::Graph::Node& impl : getImplements(graph_, subdir))
      if (subdirs.find(impl) != subdirs.end())
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, impl);
        decorateEdge(graph_, edge, implementsEdgeDecoration);
      }
      else
        graph_.delNode(impl);

    for (const util::Graph::Node& dep : getDepends(graph_, subdir))
      if (subdirs.find(dep) != subdirs.end())
      {
        util::Graph::Edge edge = graph_.createEdge(subdir, dep);
        decorateEdge(graph_, edge, dependsEdgeDecoration);
      }
      else
        graph_.delNode(dep);
  }
}

std::string FileDiagram::getSubsystemDependencyDiagramLegend()
{
  util::LegendBuilder builder("Subsystem Dependency Diagram");

  builder.addNode("center file", centerNodeDecoration);
  builder.addNode("directory", directoryNodeDecoration);
  builder.addNode("binary file", binaryFileNodeDecoration);
  builder.addNode("source file", sourceFileNodeDecoration);
  builder.addNode("header file", headerFileNodeDecoration);
  builder.addNode("object file", objectFileNodeDecoration);

  builder.addEdge("sub directory", subdirEdgeDecoration);
  builder.addEdge("implements", implementsEdgeDecoration);
  builder.addEdge("depends on", dependsEdgeDecoration);

  return builder.getOutput();
}

std::vector<util::Graph::Node> FileDiagram::getIncludedFiles(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> include;

  _transaction([&, this]{
    IncludeResult res = _db->query<model::CppHeaderInclusion>(
      (reverse_
         ? IncludeQuery::included->id
         : IncludeQuery::includer->id) == std::stoull(node_));

    for (const auto& inclusion : res)
    {
      model::FileId fileId = reverse_
        ? inclusion.includer.object_id()
        : inclusion.included.object_id();

      core::FileInfo fileInfo;
      _projectHandler.getFileInfo(fileInfo, std::to_string(fileId));
      include.push_back(addNode(graph_, fileInfo));
    }
  });

  return include;
}

std::vector<util::Graph::Node> FileDiagram::getIncludes(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getIncludedFiles(graph_, node_);
}

std::vector<util::Graph::Node> FileDiagram::getRevIncludes(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getIncludedFiles(graph_, node_, true);
}

std::vector<util::Graph::Node> FileDiagram::getSubDirs(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> usages;

  _transaction([&, this]{
    FileResult sub = _db->query<model::File>(
      FileQuery::parent == std::stoull(node_) &&
      FileQuery::type == model::File::DIRECTORY_TYPE);

    for (const model::File& subdir : sub)
    {
      core::FileInfo fileInfo;
      _projectHandler.getFileInfo(fileInfo, std::to_string(subdir.id));

      usages.push_back(addNode(graph_, fileInfo));
    }
  });

  return usages;
}

std::vector<util::Graph::Node> FileDiagram::getImplements(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getImplementedFiles(graph_, node_);
}

std::vector<util::Graph::Node> FileDiagram::getRevImplements(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getImplementedFiles(graph_, node_, true);
}

std::vector<util::Graph::Node> FileDiagram::getImplementedFiles(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<core::FileId> implements;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
      odb::query<model::File>::parent == std::stoull(node_) &&
      odb::query<model::File>::type != model::File::DIRECTORY_TYPE
    );
    std::unordered_set<core::FileId> used;

    for (const model::File &file : contained)
    {
      auto files = getProvidedFileIds(graph_, std::to_string(file.id), reverse_);
      used.insert(files.begin(), files.end());
    }
    for (const core::FileId& fileId : used)
    {
      auto file = _db->load<model::File>(std::stoull(fileId));
      implements.push_back(std::to_string(file.get()->parent.object_id()));
    }
    std::sort(implements.begin(), implements.end());
    auto it = std::unique(implements.begin(), implements.end());
    implements.resize(it - implements.begin());
    implements.erase(std::remove_if(implements.begin(), implements.end(),
                     [&node_](const util::Graph::Node& n){ return node_ == n; }),
                     implements.end());
  });

  std::vector<util::Graph::Node> annotated;

  for (const core::FileId& fileId : implements)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, fileId);

    annotated.push_back(addNode(graph_, fileInfo));
  }
  return annotated;
}

std::vector<util::Graph::Node> FileDiagram::getDepends(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependFiles(graph_, node_);
}

std::vector<util::Graph::Node> FileDiagram::getRevDepends(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependFiles(graph_, node_, true);
}

std::vector<util::Graph::Node> FileDiagram::getDependFiles(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<core::FileId> depends;

  _transaction([&, this]{
    auto contained = _db->query<model::File>(
      odb::query<model::File>::parent == std::stoull(node_) &&
      odb::query<model::File>::type != model::File::DIRECTORY_TYPE
      );

    std::unordered_set<core::FileId> used;

    for(const model::File& file : contained)
    {
      auto files = getUsedFileIds(graph_, std::to_string(file.id), reverse_);
      used.insert(files.begin(), files.end());
    }

    for(const core::FileId& fileId : used)
    {
      auto files =_db->query<model::File>(
        odb::query<model::File>::id == std::stoull(fileId)
        );

      for(const model::File& file : files)
      {
        depends.push_back(std::to_string(file.parent.object_id()));
      }
    }

    std::sort(depends.begin(), depends.end());
    auto it = std::unique(depends.begin(), depends.end());
    depends.resize(it - depends.begin());
    depends.erase(std::remove_if(depends.begin(), depends.end(),
                     [&node_](const util::Graph::Node& n){ return node_ == n; }),
                     depends.end());
  });

  std::vector<util::Graph::Node> annotated;

  for (const core::FileId& fileId: depends)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, fileId);

    annotated.push_back(addNode(graph_, fileInfo));
  }
  return annotated;
}

std::vector<util::Graph::Node> FileDiagram::getProvides(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getProvidedFiles(graph_, node_);
}

std::vector<util::Graph::Node> FileDiagram::getRevProvides(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getProvidedFiles(graph_, node_, true);
}

std::vector<util::Graph::Node> FileDiagram::getProvidedFiles(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> depends;

  std::vector<core::FileId> fileIds = getProvidedFileIds(graph_, node_, reverse_);

  for (const core::FileId& fileId : fileIds)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, fileId);

    depends.push_back(addNode(graph_, fileInfo));
  }

  return depends;
}

std::vector<core::FileId> FileDiagram::getProvidedFileIds(
  util::Graph&,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<core::FileId> depends;

  _transaction([&, this]{
    EdgeResult res = _db->query<model::CppEdge>(
      (reverse_
       ? EdgeQuery::to->id
       : EdgeQuery::from->id) == std::stoull(node_) &&
      EdgeQuery::type == model::CppEdge::PROVIDE);

    for (const model::CppEdge& edge : res)
    {
      core::FileId fileId = reverse_ ? std::to_string(edge.from->id) : std::to_string(edge.to->id);
      depends.push_back(fileId);
    }
  });

  return depends;
}

std::vector<util::Graph::Node> FileDiagram::getContains(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> contained;

  _transaction([&, this]{
    TargetResult targets = _db->query<model::BuildTarget>(
      TargetQuery::file == std::stoull(node_));

    for (const model::BuildTarget& target : targets)
      for (const auto& source : target.action->sources)
      {
        model::FileId fileId = source.lock().load()->file->id;
        core::FileInfo fileInfo;
        _projectHandler.getFileInfo(fileInfo, std::to_string(fileId));
        contained.push_back(addNode(graph_, fileInfo));
      }
  });

  return contained;
}

std::vector<util::Graph::Node> FileDiagram::getRevContains(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> contained;
  std::set<model::FileId> files;

  _transaction([&, this]{
    SourceResult sources = _db->query<model::BuildSource>(
      SourceQuery::file == std::stoull(node_));
    for (const model::BuildSource& source : sources)
      for (const auto& target : source.action->targets)
      {
        model::FileId fileId = target.lock().load()->file->id;

        if (!files.insert(fileId).second)
          continue;

        core::FileInfo fileInfo;
        _projectHandler.getFileInfo(fileInfo, std::to_string(fileId));
        contained.push_back(addNode(graph_, fileInfo));
      }
  });

  return contained;
}

std::vector<util::Graph::Node> FileDiagram::getUsages(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getUsedFiles(graph_, node_);
}

std::vector<util::Graph::Node> FileDiagram::getRevUsages(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getUsedFiles(graph_, node_, true);
}

std::vector<util::Graph::Node> FileDiagram::getUsedFiles(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> usages;

  std::vector<core::FileId> fileIds = getUsedFileIds(graph_, node_, reverse_);

  for (const core::FileId& fileId: fileIds)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, fileId);

    usages.push_back(addNode(graph_, fileInfo));
  }

  return usages;
}

std::vector<core::FileId> FileDiagram::getUsedFileIds(
  util::Graph&,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<core::FileId> usages;

  _transaction([&, this]{
    EdgeResult res = _db->query<model::CppEdge>(
      (reverse_
       ? EdgeQuery::to->id
       : EdgeQuery::from->id) == std::stoull(node_) &&
      EdgeQuery::type == model::CppEdge::USE);

    for (const model::CppEdge& edge : res)
    {
      core::FileId fileId = reverse_ ? std::to_string(edge.from->id) : std::to_string(edge.to->id);

      usages.push_back(fileId);
    }
  });

  return usages;
}

util::Graph::Node FileDiagram::addNode(
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
  else if (fileInfo_.type == "CPP")
  {
    std::string ext = boost::filesystem::extension(fileInfo_.path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".cpp" || ext == ".cxx" || ext == ".cc"  || ext == ".c")
      decorateNode(graph_, node, sourceFileNodeDecoration);
    else if (ext == ".hpp" || ext == ".hxx" || ext == ".hh"  || ext == ".h")
      decorateNode(graph_, node, headerFileNodeDecoration);
    else if (ext == ".o"  || ext == ".so" || ext == ".dll")
      decorateNode(graph_, node, objectFileNodeDecoration);
  }

  return node;
}

void FileDiagram::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void FileDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

std::string FileDiagram::getLastNParts(const std::string& path_, std::size_t n_)
{
  if (path_.empty() || n_ == 0)
    return "";

  std::size_t p;
  for (p = path_.rfind('/');
       --n_ > 0 && p - 1 < path_.size();
       p = path_.rfind('/', p - 1));

  return p > 0 && p < path_.size() ? "..." + path_.substr(p) : path_;
}

const FileDiagram::Decoration FileDiagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const FileDiagram::Decoration FileDiagram::sourceFileNodeDecoration = {
  {"shape", "box"},
  {"style", "filled"},
  {"fillcolor", "#116db6"},
  {"fontcolor", "white"}
};

const FileDiagram::Decoration FileDiagram::headerFileNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "#ef4756"},
  {"fontcolor", "white"}
};

const FileDiagram::Decoration FileDiagram::binaryFileNodeDecoration = {
  {"shape", "box3d"},
  {"style", "filled"},
  {"fillcolor", "#f18a21"},
  {"fontcolor", "white"}
};

const FileDiagram::Decoration FileDiagram::objectFileNodeDecoration = {
  {"shape", "folder"},
  {"style", "filled"},
  {"fillcolor", "#6fc59e"},
  {"fontcolor", "white"}
};

const FileDiagram::Decoration FileDiagram::directoryNodeDecoration = {
  {"shape", "folder"}
};

const FileDiagram::Decoration FileDiagram::usagesEdgeDecoration = {
  {"label", "uses"}
};

const FileDiagram::Decoration FileDiagram::revUsagesEdgeDecoration = {
  {"label", "used by"}
};

const FileDiagram::Decoration FileDiagram::providesEdgeDecoration = {
  {"label", "provides"},
  {"color", "red"}
};

const FileDiagram::Decoration FileDiagram::revProvidesEdgeDecoration = {
  {"label", "provided by"},
  {"color", "red"}
};

const FileDiagram::Decoration FileDiagram::containsEdgeDecoration = {
  {"label", "contains"},
  {"color", "blue"}
};

const FileDiagram::Decoration FileDiagram::revContainsEdgeDecoration = {
  {"label", "contained by"},
  {"color", "blue"}
};

const FileDiagram::Decoration FileDiagram::subdirEdgeDecoration = {
  {"label", "subdir"},
  {"color", "green"}
};

const FileDiagram::Decoration FileDiagram::implementsEdgeDecoration = {
  {"label", "implements"},
  {"color", "red"}
};

const FileDiagram::Decoration FileDiagram::revImplementsEdgeDecoration = {
  {"label", "implemented by"},
  {"color", "red"}
};

const FileDiagram::Decoration FileDiagram::dependsEdgeDecoration = {
  {"label", "depends on"},
  {"color", "blue"}
};

const FileDiagram::Decoration FileDiagram::revDependsEdgeDecoration = {
  {"label", "dependent on"},
  {"color", "blue"}
};

} // language
} // service
} // cc
