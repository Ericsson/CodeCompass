#include <parser/plugin_handler.h>

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
  
PluginHandler::PluginHandler(ParserContext& ctx_): _ctx(ctx_)
{
}

bool PluginHandler::loadPluginsFromDir(const std::string& path_)
{
  namespace fs = ::boost::filesystem;

  if (!fs::exists(path_) || !fs::is_directory(path_))
  {
    BOOST_LOG_TRIVIAL(error) << path_ << " is not a directory";
    return false;
  }

  fs::directory_iterator endIter;
  for (fs::directory_iterator dirIter(path_); dirIter != endIter;
    ++dirIter)
  {
    if (fs::is_regular_file(dirIter->status())
      && fs::extension(*dirIter) == util::DynamicLibrary::extension())
    {        
      std::string dynamicLibraryPath = dirIter->path().string();
      _dynamicLibraries.emplace_back(util::DynamicLibraryPtr(
        new util::DynamicLibrary(dynamicLibraryPath)));
    }
  }

  for(const util::DynamicLibraryPtr& lib : _dynamicLibraries)
  {
    typedef std::shared_ptr<AbstractParser> (*makeParser)(ParserContext& _ctx);
    auto make = reinterpret_cast<makeParser>(lib->getSymbol("make"));    
    std::shared_ptr<AbstractParser> parser = make(_ctx);    
    _parsers[parser->getName()] = parser;
  }  
  return true;
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
  std::map<std::string, 
          boost::adjacency_list<>::vertex_descriptor> parserNameToVertex;
  
  //--- Init data ---//
  for(const auto& parser : _parsers)
  {        
    vertexNames.push_back(parser.first); 
    parserNameToVertex[parser.first] = boost::add_vertex(g);
  }

  //--- Add edges ---//
  for(const auto& parser: _parsers)
  {
    auto depenncies = parser.second->getDependentParsers();
    for(const auto& dependency : depenncies)
    {
      if(!parserNameToVertex.count(parser.first))
      {
        BOOST_LOG_TRIVIAL(error) << parser.first << " is not a real parser";
        break;
      }
      if(!parserNameToVertex.count(dependency))
      {
        BOOST_LOG_TRIVIAL(error) << dependency << " is not a real parser";
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