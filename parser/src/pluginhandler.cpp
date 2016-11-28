#include <parser/pluginhandler.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include <iostream>

namespace cc
{
namespace parser
{

PluginHandler::PluginHandler(const std::string& pluginDir_)
  : _pluginDir(pluginDir_)
{
}

bool PluginHandler::loadPlugins(std::vector<std::string>& skipParserList_)
{
  namespace fs = ::boost::filesystem;

  if (!fs::exists(_pluginDir) || !fs::is_directory(_pluginDir))
  {
    BOOST_LOG_TRIVIAL(error) << _pluginDir << " is not a directory";
    return false;
  }

  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(_pluginDir); dirIter != endIter; ++dirIter)
  {
    if (fs::is_regular_file(dirIter->status()) &&
        fs::extension(*dirIter) == util::DynamicLibrary::extension())
    {
      std::string filename = dirIter->path().stem().string(); // filename without extension
      filename.erase(filename.begin(), filename.begin() + 3); // remove lib from filename
      if (std::find(skipParserList_.begin(), skipParserList_.end(),
         filename) == skipParserList_.end())
      {
        std::string dynamicLibraryPath = dirIter->path().string();
        _dynamicLibraries[filename] = util::DynamicLibraryPtr(
          new util::DynamicLibrary(dynamicLibraryPath));
      }
      else
      {
        BOOST_LOG_TRIVIAL(info) << "[" << filename << "] skipped!";
      }
    }
  }

  return true;
}

bool PluginHandler::createPlugins(ParserContext& ctx_)
{
  for (const auto& lib : _dynamicLibraries)
  {
    typedef std::shared_ptr<AbstractParser> (*makeParser)(ParserContext& _ctx);
    auto make = reinterpret_cast<makeParser>(lib.second->getSymbol("make"));
    std::shared_ptr<AbstractParser> parser = make(ctx_);
    _parsers[lib.first] = parser;
  }
  return true;
}

boost::program_options::options_description PluginHandler::getOptions() const
{
  namespace po = ::boost::program_options;

  po::options_description desc("Options of plugins");
  for (const auto& lib : _dynamicLibraries)
  {
    typedef po::options_description (*GetOptsFuncPtr)();

    GetOptsFuncPtr getOptions = reinterpret_cast<GetOptsFuncPtr>(
      lib.second->getSymbol("getOptions"));

    desc.add(getOptions());
  }

  return desc;
}

std::vector<std::string> PluginHandler::getTopologicalOrder()
{
  std::vector<std::string> topologicalOrder;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, 
          boost::bidirectionalS> Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::pair<int,int> Edge;

  Graph g;
  std::vector<Edge> edges;

  std::vector<std::string> vertexNames;
  std::map<std::string, boost::adjacency_list<>::vertex_descriptor>
    parserNameToVertex;
  
  //--- Init data ---//

  for(const auto& parser : _parsers)
  {        
    vertexNames.push_back(parser.first); 
    parserNameToVertex[parser.first] = boost::add_vertex(g);
  }

  //--- Add edges ---//

  for(const auto& parser : _parsers)
  {
    for(const auto& dependency : parser.second->getDependentParsers())
    {
      if(!parserNameToVertex.count(dependency))
      {
        // TODO: This shouldn't be tolerated so easy.
        BOOST_LOG_TRIVIAL(warning) << dependency << " is not a real parser";
        continue;
      }

      //--- Add edges to the graph ---//

      bool inserted;
      boost::graph_traits<Graph>::edge_descriptor e; 
      boost::tie(e, inserted) = boost::add_edge(
        parserNameToVertex.at(dependency), 
        parserNameToVertex.at(parser.first), g);
    }      
  } 

  //--- Topological order of the graph ---//

  std::deque<Vertex> make_order;
  boost::topological_sort(g, std::front_inserter(make_order));

  for (auto it = make_order.begin(); it != make_order.end(); ++it) 
  {
    topologicalOrder.push_back(vertexNames[*it]);
  }

  return topologicalOrder;
}

std::shared_ptr<AbstractParser>& PluginHandler::getParser(
  std::string parserName_)
{
  return _parsers.at(parserName_);
}

PluginHandler::~PluginHandler()
{
  _parsers.clear();
}

}
}
