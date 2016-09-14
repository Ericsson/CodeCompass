// $Id$
// Created by Aron Barath, 2013

#include <iostream>
#include <queue>
#include <cassert>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/tracer.hxx>

#include <util/streamlog.h>
#include "util/standarderrorlogstrategy.h"

#include <util/graph.h>
#include <util/util.h>
#include <util/diagram/legendbuilder.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/java/javaastnode-odb.hxx>
#include <model/java/javafunction.h>
#include <model/java/javafunction-odb.hxx>
#include <model/java/javatype.h>
#include <model/java/javatype-odb.hxx>
#include <model/java/javamember.h>
#include <model/java/javamember-odb.hxx>
#include <model/java/javadoccomment.h>
#include <model/java/javadoccomment-odb.hxx>
#include <model/java/javaannotation.h>
#include <model/java/javaannotation-odb.hxx>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildtarget.h>
#include <model/buildtarget-odb.hxx>
#include <model/buildsource.h>
#include <model/buildsource-odb.hxx>

#include "javaservicehelper/javaservicehelper.h"
#include "service/cppservice/src/cppservice.h"

#include "javaservicehelper/symbolhandler.h"
#include "utils.h"

namespace cc
{
namespace service
{
namespace language
{

JavaServiceHelper::JavaServiceHelper(const std::shared_ptr<odb::database>& db_)
  : db(db_), transaction(db_), query(db_)
{
}

AstNodeInfo JavaServiceHelper::getAstNodeInfoByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  return transaction([&, this]()
  {
    auto min = selectProperAstNode(queryAstNodesByPosition(fpos, filters));

    return createAstNodeInfo(min);
  });
}

model::JavaAstNode JavaServiceHelper::selectProperAstNode(odb::result<model::JavaAstNode> nodes)
{
  auto it = nodes.begin();

  model::JavaAstNode min = *it;

//  if(isClickable(min))
//    SLog() << "$$$$$ " << min.astValue << " " << min.id;

  model::Range minRange = JavaOdbQuery::getRange(min);

  for(++it; it != nodes.end(); ++it)
  {
    const auto& actual = *it;
    const auto& rng = JavaOdbQuery::getRange(actual);

//    if(isClickable(actual))
//      SLog() << "$$$$$ " << actual.astValue << " " << it->id;

    if((rng < minRange && isClickable(actual))
      || (!isClickable(min) && isClickable(actual)))
    {
      min = actual;
      minRange = rng;
    }
  }

  return min;
}

namespace diagram
{

enum class InheritanceQuery
{
  GET_BASE,
  GET_DERIVED
};

enum class Direction
{
  BASE,
  DERIVED,
  BOTH
};

typedef std::map<std::pair<std::string, bool>, util::Graph::Node> edges_t;
typedef std::map<model::JavaAstNodeId, util::Graph::Node> nodes_t;

char getVisibility(int modifiers)
{
  if(modifiers & model::JavaModifiers::Public)    { return '+'; }
  if(modifiers & model::JavaModifiers::Private)   { return '-'; }
  if(modifiers & model::JavaModifiers::Protected) { return '#'; }
  return '~'; // package private
}

std::string extractAstValue(const std::string & astValue)
{
  std::size_t start = astValue.find(' ');
  std::size_t depth = 0;

  for(std::size_t i=0,n=astValue.length();i!=n;++i)
  {
    if('<'==astValue[i])
    {
      ++depth;
    }
    else
    if('>'==astValue[i])
    {
      --depth;
    }
    else
    if(0==depth && '.'==astValue[i])
    {
      start = i;
    }
  }

  if(std::string::npos==start)
  {
    return astValue;
  }

  return astValue.substr(start+1);
}

std::string extractType(std::shared_ptr<model::JavaType> typePtr)
{
  if(typePtr)
  {
    //return " : " + typePtr->name;
    return " : " + typePtr->qualifiedName;
  }
  else
  {
    return "";
  }
}

#define PROPERTY(_name_, _content_) \
  static const std::string & _name_() \
  { \
    static const std::string content(_content_); \
    return content; \
  }

class ClassDiagramHelper
{
  std::shared_ptr<odb::database> db;
  JavaOdbQuery & query;
  util::Graph graph;
  edges_t edges;

  PROPERTY(getClassShape, "record");
  PROPERTY(getInheritanceArrowhead, "empty");

public:

  ClassDiagramHelper(std::shared_ptr<odb::database> db, JavaOdbQuery & query)
    : db(db)
    , query(query)
  {
    graph.setAttribute("rankdir", "BT");
  }

  static std::string getLegend()
  {
    static std::string legend("");

    if(!legend.empty())
    {
      return legend;
    }

    util::diagram::LegendBuilder builder;
    builder.addNode("class", { {"shape", getClassShape()} });
    builder.addEdge("inheritance", { {"arrowhead", getInheritanceArrowhead()} });
    return legend = builder.getOutput();
  }

  void build(const std::string & astNodeId)
  {
    model::JavaAstNode astNode = query.loadAstNode(astNodeId);
    model::JavaAstNode thisAstNode = query.loadDefinition(astNode);
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(
      thisAstNode.mangledNameHash);

    // Node for current class

    util::Graph::Node thisNode = graph.addNode();
    graph.setAttribute(thisNode, "id", std::to_string(thisAstNode.id));
    graph.setAttribute(thisNode, "label", getNodeLabel(thisAstNode, javaType));
    graph.setAttribute(thisNode, "shape", getClassShape());

    // Nodes for inheritance classes

    makeInheritanceNodes(javaType, diagram::InheritanceQuery::GET_BASE);
    makeInheritanceNodes(javaType, diagram::InheritanceQuery::GET_DERIVED);

    // Add edges

    for(auto it : edges)
    {
      util::Graph::Edge edge = it.first.second
                             ? graph.addEdge(thisNode, it.second)
                             : graph.addEdge(it.second, thisNode);
      graph.setAttribute(edge, "arrowhead", getInheritanceArrowhead());
    }
  }

  std::string getOutput()
  {
    return graph.output(util::Graph::SVG);
  }

private:

  std::string getNodeLabel(const model::JavaAstNode& node, const model::JavaType& javaType)
  {
    std::string label = '{' + extractAstValue(node.astValue) + '|';

    for(const odb::lazy_shared_ptr<model::JavaMember>& member : javaType.fields)
    {
      label += getVisibility(member.load()->modifiers)
            +  extractAstValue(member.load()->astNodePtr.load()->astValue)
            +  extractType(member.load()->fieldType.load())
            +  "\\l";
    }
    label += '|';

    for(const odb::lazy_weak_ptr<model::JavaFunction>& func : javaType.functions)
    {
      label += getVisibility(func.load()->modifiers)
            +  extractAstValue(func.load()->astNodePtr.load()->astValue) + "()"
            +  extractType(func.load()->returnType.load())
            +  "\\l";
    }

    label += '}';

    return escapeDot(label);
  }

  void makeInheritanceNodes(model::JavaType & javaType, InheritanceQuery iquery)
  {
    const bool out = InheritanceQuery::GET_BASE==iquery;
    std::vector<model::JavaType> types;

    if(InheritanceQuery::GET_BASE==iquery)
    {
      types = std::move(getInheritsFromTypes(db, query, javaType.mangledNameHash));
    }
    else
    {
      types = std::move(getInheritsByTypes(db, query, javaType.mangledNameHash));
    }

    for(model::JavaType & type : types)
    {
      std::string nodeLabel;
      std::string nodeId;

      if(type.astNodePtr.load())
      {
        model::JavaAstNode otherAstNode = *type.astNodePtr.load();
        // in case of generic base, the generic type is shown instead of
        // specified one
        nodeLabel = getNodeLabel(
          otherAstNode, (type.genericImpl) ? *type.genericImpl.load() : type);
        nodeId = std::to_string(otherAstNode.id);
      }
      else
      {
        nodeLabel = std::string("[") + escapeDot(type.qualifiedName) + "]";
        nodeId = std::string("ext_") + std::to_string(type.mangledNameHash);
      }

      std::pair<std::string, bool> p(nodeLabel, out);

      util::Graph::Node otherNode = graph.addNode();
      graph.setAttribute(otherNode, "label", nodeLabel);
      graph.setAttribute(otherNode, "id", nodeId);
      graph.setAttribute(otherNode, "shape", getClassShape());
      edges[p] = otherNode;
    }
  }
};

//----------------------------------------------------------------

class FullClassDiagramHelper
{
  std::shared_ptr<odb::database> db;
  JavaOdbQuery & query;
  util::Graph graph;

  nodes_t visitedNodes;

  PROPERTY(getNodeShape, "box");
  PROPERTY(getCurrentNodeStyle, "filled");
  PROPERTY(getCurrentNodeFillcolor, "gold");
  PROPERTY(getInheritanceArrowhead, "empty");
  PROPERTY(getAggregationArrowhead, "diamond");

public:

  FullClassDiagramHelper(std::shared_ptr<odb::database> db, JavaOdbQuery & query)
    : db(db)
    , query(query)
  {
    graph.setAttribute("rankdir", "LR");
  }

  static std::string getLegend()
  {
    static std::string legend("");

    if(!legend.empty())
    {
      return legend;
    }

    util::diagram::LegendBuilder builder;
    builder.addNode("current class", { {"shape", getNodeShape()},
      {"style", getCurrentNodeStyle()}, {"fillcolor", getCurrentNodeFillcolor()} });
    builder.addNode("class", { {"shape", getNodeShape()} });
    builder.addEdge("inheritance", { {"arrowhead", getInheritanceArrowhead()} });
    builder.addEdge("aggregation", { {"arrowhead", getAggregationArrowhead()} });
    return legend = builder.getOutput();
  }

  void build(const std::string & astNodeId)
  {
    model::JavaAstNode astNode = query.loadAstNode(astNodeId);
    model::JavaAstNode thisAstNode = query.loadDefinition(astNode); 

    createNode(thisAstNode, true);
    collectNeighbours(thisAstNode, diagram::Direction::BOTH);
    collectAggregation(thisAstNode);
  }

  std::string getOutput()
  {
    return graph.output(util::Graph::SVG);
  }

private:

  util::Graph::Node createNode(const model::JavaAstNode & astNode, bool current=false)
  {
    return visitedNodes[{astNode.id}] = createNode(extractAstValue(astNode.astValue),
      std::to_string(astNode.id), current);
  }

  util::Graph::Node createNode(const model::JavaType & type, bool current=false)
  {
    return createNode(std::string("[") + type.qualifiedName + "]",
            std::string("ext_") + std::to_string(type.mangledNameHash), current);
  }

  util::Graph::Node createNode(const std::string & nodeLabel, const std::string & nodeId,
    bool current=false)
  {
    util::Graph::Node node = graph.addNode();

    graph.setAttribute(node, "id", nodeId);
    graph.setAttribute(node, "shape", getNodeShape());
    graph.setAttribute(node, "label", nodeLabel);

    if(current)
    {
      graph.setAttribute(node, "style", getCurrentNodeStyle());
      graph.setAttribute(node, "fillcolor", getCurrentNodeFillcolor());
    }

    return node;
  }

  void collectNeighbours(const model::JavaAstNode & node, Direction direction=Direction::BOTH)
  {
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(node.mangledNameHash);

    // find bases

    if(Direction::DERIVED!=direction) // BASE or BOTH
    {
      for(const model::JavaType & base : getInheritsFromTypes(db, query, javaType.mangledNameHash))
      {
        model::JavaAstNodePtr classAstNode = base.astNodePtr.load();
        util::Graph::Node newNode;

        if(classAstNode)
        {
          if(visitedNodes.find({classAstNode->id})==visitedNodes.end())
          {
            newNode = createNode(*classAstNode);
          }
          else
          {
            continue;
          }
        }
        else
        {
          newNode = createNode(base);
        }

        util::Graph::Edge edge = graph.addEdge(visitedNodes[{node.id}], newNode);
        graph.setAttribute(edge, "arrowhead", getInheritanceArrowhead());

        if(classAstNode)
        {
          collectNeighbours(*classAstNode, Direction::BASE);
        }
      }
    }

    // find derived

    if(Direction::BASE!=direction) // DERIVED or BOTH
    {
      for(const model::JavaType & derived : getInheritsByTypes(db, query, javaType.mangledNameHash))
      {
        model::JavaAstNodePtr classAstNode = derived.astNodePtr.load();
        util::Graph::Node newNode;

        if(classAstNode)
        {
          if(visitedNodes.find({classAstNode->id})==visitedNodes.end())
          {
            newNode = createNode(*classAstNode);
          }
          else
          {
            continue;
          }
        }
        else
        {
          newNode = createNode(derived);
        }

        util::Graph::Edge edge = graph.addEdge(newNode, visitedNodes[{node.id}]);
        graph.setAttribute(edge, "arrowhead", getInheritanceArrowhead());

        if(classAstNode)
        {
          collectNeighbours(*classAstNode, Direction::DERIVED);
        }
      }
    }
  }

  void collectAggregation(const model::JavaAstNode & node)
  {
    model::JavaType javaType = query.queryEntityByHash<model::JavaType>(node.mangledNameHash);

    for(odb::lazy_shared_ptr<model::JavaMember> member : javaType.fields)
    {
      try
      {
        if(member.load() && member->fieldType.load() && member->fieldType->astNodePtr.load())
        {
          model::JavaAstNode & typeAstNode = *member->fieldType->astNodePtr.load();

          if(!typeAstNode.astValue.empty() && visitedNodes.find({typeAstNode.id})==visitedNodes.end())
          {
            util::Graph::Edge edge = graph.addEdge(createNode(typeAstNode), visitedNodes[{node.id}]);
            graph.setAttribute(edge, "arrowhead", getAggregationArrowhead());
          }
        }
      }
      catch(const std::exception & ex)
      {
        SLog(util::ERROR) << ex.what();
      }
    }
  }
};

//----------------------------------------------------------------

class FunctionCallDiagramHelper
{
  std::shared_ptr<odb::database> db;
  JavaOdbQuery & query;
  util::Graph graph;
  std::map<std::string, util::Graph::Subgraph> files;
  std::map<std::pair<model::JavaAstNode, std::string>, util::Graph::Node> graphNode;
  util::Graph::Node centerGraphNode;

  PROPERTY(getGlobalShape, "star");
  PROPERTY(getCenterNodeStyle, "filled");
  PROPERTY(getCenterNodeFillcolor, "gold");
  PROPERTY(getCallNodeStyle, "filled");
  PROPERTY(getCallNodeIncolor, "coral");
  PROPERTY(getCallNodeOutcolor, "lightblue");
  PROPERTY(getOverridableShape, "diamond");
  PROPERTY(getOverridableStyle, "filled");
  PROPERTY(getOverridableFillcolor, "cyan");
  PROPERTY(getFinalShape, "ellipse");
  PROPERTY(getFinalStyle, "filled");
  PROPERTY(getFinalFillcolor, "lightblue");
  PROPERTY(getCalledEdgeColor, "blue");
  PROPERTY(getCallerEdgeColor, "red");
  PROPERTY(getOverriderEdgeStyle, "dashed");

public:

  FunctionCallDiagramHelper(std::shared_ptr<odb::database> db, JavaOdbQuery & query)
    : db(db)
    , query(query)
  {
    graph.setAttribute("rankdir", "LR");
  }

  static std::string getLegend()
  {
    static std::string legend("");

    if(!legend.empty())
    {
      return legend;
    }

    util::diagram::LegendBuilder builder;
    builder.addNode("center function", { {"style", getCenterNodeStyle()},
      {"fillcolor", getCenterNodeFillcolor()} });
    builder.addNode("global function", { {"shape", getGlobalShape()} });
    builder.addNode("static called", { {"style", getCallNodeStyle()},
      {"fillcolor", getCallNodeOutcolor()} });
    builder.addNode("static caller", { {"style", getCallNodeStyle()},
      {"fillcolor", getCallNodeIncolor()} });
    builder.addNode("overridable function", { {"shape", getOverridableShape()},
      {"style", getOverridableStyle()}, {"fillcolor", getOverridableFillcolor()} });
    builder.addNode("final function", { {"shape", getFinalShape()},
      {"style", getFinalStyle()}, {"fillcolor", getFinalFillcolor()} });
    builder.addEdge("called", { {"color", getCalledEdgeColor()} });
    builder.addEdge("caller", { {"color", getCallerEdgeColor()} });
    builder.addEdge("overrider", { {"style", getOverriderEdgeStyle()} });
    return legend = builder.getOutput();
  }

  void build(const std::string & astNodeId)
  {
    model::JavaAstNode astNode = query.loadAstNode(astNodeId);
    model::JavaAstNode centerNode = query.loadDefinitionOrDeclaration(astNode);

    addCenterGraphNode(centerNode);

    staticCalls(db->query(JavaOdbQuery::queryStaticCallsInAstNode(centerNode)), true);
    staticCalls(db->query(JavaOdbQuery::queryStaticCallers(centerNode)), false);
    dynamicCalls(db->query(JavaOdbQuery::queryDynamicCallsInAstNode(centerNode)));
    dynamicCallers(centerNode);
  }

  std::string getOutput()
  {
    return graph.output(util::Graph::SVG);
  }

private:

  void addCenterGraphNode(const model::JavaAstNode & centerNode)
  {
    centerGraphNode = addNode(centerNode, getNodeLabel(centerNode));
    graph.setAttribute(centerGraphNode, "style", getCenterNodeStyle());
    graph.setAttribute(centerGraphNode, "fillcolor", getCenterNodeFillcolor());
  }

  std::string addFileSubgraphIfNeeded(const model::JavaAstNode & astNode)
  {
    std::string filename;

    if(astNode.file)
    {
      filename = astNode.file.load()->path;
    }
    else
    {
      return filename;
    }

    if(files.find(filename)==files.end())
    {
      util::Graph::Subgraph fileGraph = graph.addSubgraph("cluster_" + filename);
      graph.setAttribute(fileGraph, "label", filename);
      files[filename] = fileGraph;
    }

    return filename;
  }

  std::string getNodeLabel(const model::JavaAstNode & astNode)
  {
    return extractAstValue(astNode.astValue);
  }

  util::Graph::Node addNode(const model::JavaAstNode & astNode, const std::string & label)
  {
    auto nodeAndLabel = std::make_pair(astNode, label);
    auto it = graphNode.find(nodeAndLabel);
    if(graphNode.end()!=it)
    {
      return it->second;
    }

    util::Graph::Node node = graph.addNode(
      files[addFileSubgraphIfNeeded(astNode)]);

    graph.setAttribute(node, "label", label);
    graph.setAttribute(node, "id", std::to_string(astNode.id));

    graphNode[nodeAndLabel] = node;

    return node;
  }

  void addCallColor(const util::Graph::Node & node, bool called)
  {
    graph.setAttribute(node, "style", getCallNodeStyle());

    const std::string & fillcolor = called ? getCallNodeOutcolor() : getCallNodeIncolor();

    const std::string & clr = graph.getAttribute(node, "fillcolor");
    if(!clr.empty() && clr!=getCallNodeOutcolor() && clr!=getCallNodeIncolor())
    {
      return;
    }

    if(clr.empty())
    {
      graph.setAttribute(node, "fillcolor", fillcolor);
    }
    else
    {
      graph.setAttribute(node, "fillcolor", getCallNodeIncolor() + ":" + getCallNodeOutcolor());
    }
  }

  void staticCalls(odb::result<model::JavaAstNode> calls, bool out)
  {
    std::set<JavaHashType> visited;

    for(model::JavaAstNode & staticCall : calls)
    {
      try
      {
        model::JavaAstNode defNode = out ?
          query.loadDefinitionOrDeclaration(staticCall) :
          query.loadOuterFunction(staticCall);

        if(!visited.insert(defNode.mangledNameHash).second)
        {
          continue;
        }

        util::Graph::Node callGraphNode = addNode(defNode, getNodeLabel(defNode));
        addCallColor(callGraphNode, out);

        util::Graph::Edge edge;
        if(out)
        {
          edge = graph.addEdge(centerGraphNode, callGraphNode);
        }
        else
        {
          edge = graph.addEdge(callGraphNode, centerGraphNode);
        }

        graph.setAttribute(edge, "color", out ? getCalledEdgeColor() : getCallerEdgeColor());
      }
      catch(const JavaOdbQuery::NoOuterFunction & ex)
      {
        util::Graph::Node node = addNode(staticCall, "Global Scope");
        addCallColor(node, false);

        graph.setAttribute(node, "shape", getGlobalShape());

        util::Graph::Edge edge = graph.addEdge(node, centerGraphNode);
        graph.setAttribute(edge, "color", getCallerEdgeColor());
      }
      catch(const std::runtime_error & ex)
      {
        SLog(util::ERROR) << "Exception: " << ex.what();
      }
    }
  }

  void dynamicCalls(odb::result<model::JavaAstNode> calls)
  {
    std::set<JavaHashType> visited;

    for(const model::JavaAstNode & node : calls)
    {
      try
      {
        if(!visited.insert(node.mangledNameHash).second)
        {
          continue;
        }

        model::JavaAstNode definition = query.loadDefinitionOrDeclaration(node);

        std::string nodeLabel = getNodeLabel(definition);

        util::Graph::Node callGraphNode = addNode(definition, nodeLabel);

        setStyleOverridable(callGraphNode);

        try
        {
          model::JavaFunction func = query.queryEntityByHash<model::JavaFunction>(definition.mangledNameHash);

          if(func.modifiers & model::JavaModifiers::Final)
          {
            setStyleFinal(callGraphNode);
          }
		}
        catch(...)
        {
          // we do not care what exception has thrown
        }

        util::Graph::Edge edge = graph.addEdge(centerGraphNode, callGraphNode);
        graph.setAttribute(edge, "color", getCalledEdgeColor());

        std::set<JavaHashType> visited;
        visited.insert(node.mangledNameHash);

        for(JavaHashType overrider : getOverrideHashes(db, query,
          query.queryEntityByHash<model::JavaFunction>(node.mangledNameHash)))
        {
          if(!visited.insert(overrider).second)
          {
            continue;
          }

          model::JavaFunction overriderFunc = query.queryEntityByHash<model::JavaFunction>(overrider);
          model::JavaAstNode overriderNode = *overriderFunc.astNodePtr.load();

          util::Graph::Node dynGraphNode =
            addNode(overriderNode, getNodeLabel(overriderNode));
          addCallColor(dynGraphNode, true);

          util::Graph::Edge edge = graph.addEdge(callGraphNode, dynGraphNode);
          graph.setAttribute(edge, "style", getOverriderEdgeStyle());
          graph.setAttribute(edge, "color", getCalledEdgeColor());
        }
      }
      catch(std::runtime_error & err)
      {
        SLog(util::ERROR) << "Exception: " << err.what();
      }
    }
  }

  void dynamicCallers(const model::JavaAstNode & node)
  {
    std::set<JavaHashType> visited;

    for(auto overridden : getOverrideHashes(db, query,
      query.queryEntityByHash<model::JavaFunction>(node.mangledNameHash)))
    {
      try
      {
        if(!visited.insert(overridden).second)
        {
          continue;
        }

        model::JavaFunction overriddenFunc = query.queryEntityByHash<model::JavaFunction>(overridden);
        if(!overriddenFunc.astNodePtr.load()) { continue; }
        model::JavaAstNode overriddenNode = *overriddenFunc.astNodePtr.load();

        auto dynamicCallerOfOverridden =
          db->query(query.queryDynamicCallers(overriddenNode));

        std::set<JavaHashType> visited;

        for(auto dynamicCaller : dynamicCallerOfOverridden)
        {
          model::JavaAstNode caller = query.loadOuterFunction(dynamicCaller);

          if(!visited.insert(caller.mangledNameHash).second)
          {
            continue;
          }

          util::Graph::Node callerGraphNode = addNode(caller, getNodeLabel(caller));
          addCallColor(callerGraphNode, false);

          util::Graph::Edge edge = graph.addEdge(callerGraphNode, centerGraphNode);
          graph.setAttribute(edge, "style", getOverriderEdgeStyle());
          graph.setAttribute(edge, "color", getCallerEdgeColor());
        }
      }
      catch(const std::runtime_error & ex)
      {
        SLog(util::ERROR) << "Exception: " << ex.what();
      }
    }
  }

  void setStyleOverridable(util::Graph::Node & node)
  {
    graph.setAttribute(node, "shape", getOverridableShape());
    graph.setAttribute(node, "style", getOverridableStyle());
    graph.setAttribute(node, "fillcolor", getOverridableFillcolor());
  }

  void setStyleFinal(util::Graph::Node & node)
  {
    graph.setAttribute(node, "shape", getFinalShape());
    graph.setAttribute(node, "style", getFinalStyle());
    graph.setAttribute(node, "fillcolor", getFinalFillcolor());
  }
};

//----------------------------------------------------------------

} // diagram

std::string JavaServiceHelper::getIncludeDependencyDiagram(const core::FileId& fileId)
{
  enum Direction {INCLUDER, INCLUDED, BOTH};
  
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  typedef std::map<model::FileId, util::Graph::Node> VisitedMap;
  VisitedMap visited;

  //--- Helper functions ---//

  auto createNode = [this, &graph](const model::FileId& fileId, bool bold = false) {
    util::Graph::Node node = graph.addNode();
    model::File file;
    db->load<model::File>(fileId, file);
    std::string path = file.path;
    std::string extension = util::getExtension(path);
    
    graph.setAttribute(node, "id", std::to_string(fileId));
    graph.setAttribute(node, "label", file.filename);
    graph.setAttribute(node, "tooltip", path);

    if(bold)
    {
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", "gold");
    }

    if(extension == "java" ||
       extension == "JAVA")
    {
      graph.setAttribute(node, "shape", "box");
    }

    return node;
  };
  
  //--- Body ---//

  transaction([&, this]() {
    auto fid = stoull(fileId.fid);
    visited[fid] = createNode(fid, true);
  });

  return graph.output(util::Graph::SVG);
}

std::string JavaServiceHelper::getBuildDiagramLocal(const core::FileId& fileId)
{
  std::vector<BuildDiagramRule> rules;
  rules.push_back(BuildDiagramRule(0, ALL));
  
  return getBuildDiagram(fileId, rules);
}

std::string JavaServiceHelper::getBuildDiagramComponent(const core::FileId& fileId)
{
  std::vector<BuildDiagramRule> rules;
  rules.push_back(BuildDiagramRule(0, 0, CONTAINS));
  rules.push_back(BuildDiagramRule(0, 1, USES));
  rules.push_back(BuildDiagramRule(1, 2, PROVIDEDBY));
  rules.push_back(BuildDiagramRule(2, 2, CONTAINEDBY));

  return getBuildDiagram(fileId, rules);
}

std::string JavaServiceHelper::getBuildDiagram(const core::FileId& fileId, const std::vector<BuildDiagramRule>& rules)
{
  struct QueueItem
  {
    model::FileId file;
    int level;
    
    QueueItem(model::FileId file_, int level_ = 0)
        : file(file_), level(level_)
    { }
  };
  
  util::Graph graph;
  graph.setAttribute("rankdir", "RL");
  
  std::queue<QueueItem> q;
  
  typedef std::map<model::FileId, util::Graph::Node> NodeMap;
  NodeMap nodes;

  //--- Helper functions ---//
  
  auto createNode = [this, &graph](const model::FileId& fileId, bool bold = false) {
    util::Graph::Node node = graph.addNode();
    model::File file;
    db->load<model::File>(fileId, file);
    std::string path = file.path;
    std::string extension = util::getExtension(path);

    graph.setAttribute(node, "id", std::to_string(fileId));
    graph.setAttribute(node, "label", file.filename);
    graph.setAttribute(node, "tooltip", path);
    
    std::string shape, color;
    // Some binary files have no extension.
    // Since util::getFilename and util::getExtension both returns the filename 
    // for files without extension, it's quite hard to decide whether a file has
    // a real extension or not.
    if (path == util::getPathAndFileWithoutExtension(path) + "/" + util::getFilename(path) ||
        extension == "out" || extension == "OUT"){
      shape = "box3d";
      color = "orange";
    }
    else if (extension == "cpp" || extension == "CPP" ||
        extension == "cxx" || extension == "CXX" ||
        extension == "cc"  || extension == "CC"  ||
        extension == "c"   || extension == "C") {
      shape = "box";
      color = "#afcbe4"; // Blue
    }
    else if(extension == "h"    || extension == "H" ||
            extension == "hpp"  || extension == "HPP" ||
            extension == "hxx"  || extension == "HXX") {
      color = "#e4afaf"; // Red
    }
    else if (extension == "o"  || extension == "O" ||
             extension == "so" || extension == "SO"){
      shape = "folder";
      color = "#afe4bf"; // Green
    }
    
    if(shape.length() > 0)
      graph.setAttribute(node, "shape", shape);
    if(color.length() > 0) {
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", color);
    }
    
    if (bold) {
      if(color.length() == 0)
        graph.setAttribute(node, "style", "bold");
      else {
        graph.setAttribute(node, "style", "filled, bold");
        graph.setAttribute(node, "fillcolor", color);
      }
    }

    return node;
  };
  
  auto collectNeighbours = [&, this](const model::FileId& current, int level) {
    for(const BuildDiagramRule& rule : rules) {
      if(rule.level != level)
        continue;
      
      if(rule.relation & CONTAINEDBY) {
        typedef odb::result<model::BuildSource> sourceResult;
        typedef odb::query<model::BuildSource> sourceQuery;

        sourceResult sr (db->query<model::BuildSource> (
          sourceQuery::file == current));
        model::BuildSource bs;
        for (sourceResult::iterator i(sr.begin()); i!=sr.end();++i) {
          i.load(bs);

          for(const auto &targetWeakPtr : bs.action->targets) {
            auto targetSharedPtr = targetWeakPtr.lock();
            targetSharedPtr.load();
            targetSharedPtr->file.load();
            auto targetId = targetSharedPtr->file->id;

            util::Graph::Node targetNode;
            NodeMap::iterator it = nodes.find(targetId);
            if (it == nodes.end())
            {
              q.push(QueueItem(targetId, rule.result));
              targetNode = createNode(targetId);
              nodes[targetId] = targetNode;
            }
            else
              targetNode = nodes[targetId];

            if (!graph.hasEdge(nodes[current], targetNode)) {
              auto targetEdge = graph.addEdge(nodes[current], targetNode);
              graph.setAttribute(targetEdge, "label", "containedby");
            }
          }
        }
      }

      if(rule.relation & CONTAINS) {
        typedef odb::result<model::BuildTarget> targetResult;
        typedef odb::query<model::BuildTarget> targetQuery;  

        targetResult tr (db->query<model::BuildTarget> (
          targetQuery::file == current));
        model::BuildTarget bt;
        for (targetResult::iterator i(tr.begin()); i!=tr.end();++i){    
          i.load(bt);

          for(const auto &sourceWeakPtr : bt.action->sources) {
            auto sourceSharedPtr = sourceWeakPtr.lock();
            sourceSharedPtr.load();
            sourceSharedPtr->file.load();
            auto sourceId = sourceSharedPtr->file->id;

            util::Graph::Node sourceNode;
            NodeMap::iterator it = nodes.find(sourceId);
            if (it == nodes.end())
            {
              q.push(QueueItem(sourceId, rule.result));
              sourceNode = createNode(sourceId);
              nodes[sourceId] = sourceNode;
            }
            else
              sourceNode = nodes[sourceId];

            if (!graph.hasEdge(sourceNode, nodes[current])) {
              auto sourceEdge = graph.addEdge(sourceNode, nodes[current]);
              graph.setAttribute(sourceEdge, "label", "contains");
              graph.setAttribute(sourceEdge, "dir", "back");
            }
          }
        }
      }

      if(rule.relation & PROVIDEDBY) {
        typedef odb::result<model::JavaAstNode> astNodeResult;
        typedef odb::query<model::JavaAstNode> astNodeQuery;

        astNodeResult decls (db->query<model::JavaAstNode> (
          astNodeQuery::file == current &&
          astNodeQuery::astType == model::AstType::Declaration));
        model::JavaAstNode decl;
        for (astNodeResult::iterator i(decls.begin()); i != decls.end(); ++i) {    
          i.load(decl);
          decl.file.load();
          auto declId = decl.file->id;

          astNodeResult defs (db->query<model::JavaAstNode>(
            astNodeQuery::mangledNameHash == decl.mangledNameHash &&
            astNodeQuery::astType == model::AstType::Definition));
          model::JavaAstNode def;
          for (astNodeResult::iterator j(defs.begin()); j != defs.end(); ++j) {
            j.load(def);
            def.file.load();
            auto defId = def.file->id;

            if(defId != declId) {
              util::Graph::Node defNode;
              NodeMap::iterator it = nodes.find(defId);
              if (it == nodes.end())
              {
                q.push(QueueItem(defId, rule.result));
                defNode = createNode(defId);
                nodes[defId] = defNode;
              }
              else
                defNode = nodes[defId];

              if (!graph.hasEdge(nodes[current], defNode)) {
                auto defEdge = graph.addEdge(nodes[current], defNode);
                graph.setAttribute(defEdge, "label", "providedby");
              }
            }
          }
        }
      }

      if(rule.relation & PROVIDES) {
        typedef odb::result<model::JavaAstNode> astNodeResult;
        typedef odb::query<model::JavaAstNode> astNodeQuery;

        astNodeResult defs (db->query<model::JavaAstNode> (
          astNodeQuery::file == current &&
          astNodeQuery::astType == model::AstType::Definition));
        model::JavaAstNode def;
        for (astNodeResult::iterator i(defs.begin()); i != defs.end(); ++i) {    
          i.load(def);
          def.file.load();
          auto defId = def.file->id;

          astNodeResult decls (db->query<model::JavaAstNode>(
            astNodeQuery::mangledNameHash == def.mangledNameHash &&
            astNodeQuery::astType == model::AstType::Declaration));
          model::JavaAstNode decl;
          for (astNodeResult::iterator j(decls.begin()); j != decls.end(); ++j)
          {
            j.load(decl);
            decl.file.load();
            auto declId = decl.file->id;

            if(declId != defId)
            {
              util::Graph::Node declNode;
              NodeMap::iterator it = nodes.find(declId);
              if (it == nodes.end())
              {
                q.push(QueueItem(declId, rule.result));
                declNode = createNode(declId);
                nodes[declId] = declNode;
              }
              else
                declNode = nodes[declId];

              if(!graph.hasEdge(declNode, nodes[current]))
              {
                auto declEdge = graph.addEdge(declNode, nodes[current]);
                graph.setAttribute(declEdge, "label", "provides");
                graph.setAttribute(declEdge, "dir", "back");
              }
            }
          }
        }
      }

    }
  };

  //--- Body ---//

  transaction([&, this]() {
    q.push(QueueItem(stoull(fileId.fid), 0));
    nodes[stoull(fileId.fid)] = createNode(stoull(fileId.fid), true);

    while (!q.empty())
    {
      QueueItem current = q.front();
      q.pop();
      collectNeighbours(current.file, current.level);
    }
  });

  return graph.output(util::Graph::SVG);
}

void JavaServiceHelper::fillInfoBox(InfoBox & ret,
  const model::JavaAstNode & javaAstNode)
{
  ret.fileType = core::FileType::JavaSource;

  JavaHashType implMangledNameHash = javaAstNode.mangledNameHash;

  if(javaAstNode.mangledNameHash)
  {
    switch(javaAstNode.symbolType)
    {
    case model::SymbolType::Type:
      {
        model::JavaType type = query.queryEntityByHash<model::JavaType>(
          javaAstNode.mangledNameHash);

        if(type.genericImpl.load())
        {
          implMangledNameHash = type.genericImpl.load()->mangledNameHash;
        }
      }
      break;

    case model::SymbolType::Function:
      {
        model::JavaFunction func = query.queryEntityByHash<model::JavaFunction>(
          javaAstNode.mangledNameHash);

        if(func.genericImpl.load())
        {
          implMangledNameHash = func.genericImpl.load()->mangledNameHash;
        }
      }
      break;

    default: break;
    }

    ret.documentation = "<b>Declaration:</b><br/>";
    for(model::JavaAnnotation & ann : db->query<model::JavaAnnotation>(
      odb::query<model::JavaAnnotation>::astNodePtr == javaAstNode.id))
    {
      // TODO: print annotation fields when they are implemented
      ret.documentation += "@" + escapeHtml(ann.name) + "<br/>";
    }

    switch(javaAstNode.symbolType)
    {
    case model::SymbolType::Function:
      {
        model::JavaFunction func = query.queryEntityByHash<model::JavaFunction>(
          javaAstNode.mangledNameHash);

        if(func.returnType.load())
        {
          ret.documentation += escapeHtml(func.returnType.load()->qualifiedName) + " ";
        }

        ret.documentation += escapeHtml(func.qualifiedName);
        ret.documentation += escapeHtml(func.signature.substr(func.signature.find_first_of('(')));
      }
      break;

    case model::SymbolType::Type:
      {
        model::JavaType type = query.queryEntityByHash<model::JavaType>(
          javaAstNode.mangledNameHash);

        ret.documentation += escapeHtml(type.qualifiedName);
      }
      break;

    case model::SymbolType::Variable:
      {
        model::JavaVariable var = query.queryEntityByHash<model::JavaVariable>(
          javaAstNode.mangledNameHash);

        if(var.type.load())
        {
          ret.documentation += escapeHtml(var.type.load()->qualifiedName) + " ";
        }

        ret.documentation += var.name;
      }
      break;

    default:
      ret.documentation.clear();
      break;
    }

    ret.documentation += "<br/>";

    // declaration added, going to get DocComment

    ret.documentation += getDocCommentInTransaction(implMangledNameHash);
  }
}

InfoBox JavaServiceHelper::getInfoBox(
  const core::AstNodeId& astNodeId)
{
  return transaction([&, this]
  {
    InfoBox ret;

    std::unique_ptr<SymbolHandler> sHandler = SymbolHandler::getHandler(db, getNodeKind(astNodeId));
    model::JavaAstNode javaAstNode
      = *db->load<model::JavaAstNode>(std::stoul(astNodeId.astNodeId));

    ret.information = sHandler->getInfoBoxText(javaAstNode);
    fillInfoBox(ret, javaAstNode);

    return ret;
  });
}

InfoBox JavaServiceHelper::getInfoBoxByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  return transaction([&, this]()
  {
    try
    {
      InfoBox ret;

      model::JavaAstNode min = selectProperAstNode(queryAstNodesByPosition(fpos, filters));
      std::unique_ptr<SymbolHandler> sHandler = SymbolHandler::getHandler(db, min.symbolType);

      ret.information = sHandler->getInfoBoxText(min);
      fillInfoBox(ret, min);

      return ret;
    }
    catch(const std::exception& ex)
    {
      SLog(util::WARNING)
        << "Exception caught: " << ex.what();
    }

    return InfoBox();
  });
}

odb::result<model::JavaAstNode> JavaServiceHelper::queryAstNodesByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  model::Position position;
  position.line = fpos.pos.line;
  position.column = fpos.pos.column;

  SLog(util::DEBUG) << "Start searching, line=" << position.line << ", col=" << position.column;

  auto result = db->query<model::JavaAstNode>(
    JavaOdbQuery::astFileId(stoull(fpos.file.fid)) && JavaOdbQuery::astContains(position));

  if(result.empty())
  {
    throw core::InvalidPosition();
  }
  return result;
}

std::string JavaServiceHelper::getSourceCode(
  const core::AstNodeId& astNodeId)
{
  std::string fileContent;
  model::Range range;
  transaction([&, this]() {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    fileContent = astNode.file.load()->content.load()->content;
    range = JavaOdbQuery::getRange(astNode);
  });

  return textRange(fileContent, range);
}

//must be in an odb transaction to call this
std::string JavaServiceHelper::getDocCommentInTransaction(
  const JavaHashType mangledNameHash_)
{
  std::string ret;

  std::vector<model::JavaDocComment> docComments =
    query.template loadDocComments<model::JavaDocComment>(mangledNameHash_);

  //TODO this is not optimal
  for (const model::JavaDocComment& dc: docComments)
  {
    ret += dc.contentHTML;
  }

  return ret;
}

std::string JavaServiceHelper::getDocComment(
  const core::AstNodeId& astNodeId)
{
  throw std::logic_error("not implemented");
}

AstNodeInfo JavaServiceHelper::getAstNodeInfo(
const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    return createAstNodeInfo(query.loadAstNode(astNodeId.astNodeId));
  });
}

std::string JavaServiceHelper::getClassDiagram(
  const core::AstNodeId& astNodeId)
{
  diagram::ClassDiagramHelper cd(db, query);

  transaction([&, this]()
  {
    cd.build(astNodeId.astNodeId);
  });

  return cd.getOutput();
}

AstNodeInfo JavaServiceHelper::getDefinition(
const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    model::JavaAstNode astNode = query.loadAstNode(astNodeId.astNodeId);
    astNode = query.loadDefinitionOrDeclaration(astNode);
    model::JavaAstNode defAstNode = astNode;

    if(model::SymbolType::Type==astNode.symbolType)
    {
      model::JavaType type = query.queryEntityByHash<model::JavaType>(
        astNode.mangledNameHash);

      if(type.genericImpl.load() && type.genericImpl->astNodePtr.load())
      {
        defAstNode = *type.genericImpl->astNodePtr.load();
      }
    }
    else
    if(model::SymbolType::Function==astNode.symbolType)
    {
      model::JavaFunction func = query.queryEntityByHash<model::JavaFunction>(
        astNode.mangledNameHash);

      if(func.genericImpl.load() && func.genericImpl->astNodePtr.load())
      {
        defAstNode = *func.genericImpl->astNodePtr.load();
      }
    }

    return createAstNodeInfo(astNode, defAstNode);
  });
}

std::vector<InfoNode> JavaServiceHelper::getInfoTree(
  const core::AstNodeId& astNodeId)
{
  return transaction([&,this]()
  {
    try
    {
      model::JavaAstNode astNode = query.loadAstNode(astNodeId.astNodeId);
      model::JavaAstNode defNode = query.loadDefinitionOrDeclaration(astNode);

      std::unique_ptr<SymbolHandler> handler = SymbolHandler::getHandler(db, defNode.symbolType);

      return handler->getInfoTree(defNode);
    }
    catch(const std::exception& ex)
    {
      SLog(util::WARNING)
        << "Exception caught: " << ex.what();
    }

    return std::vector<InfoNode>{};
  });
}

std::vector<InfoNode> JavaServiceHelper::getSubInfoTree(
  const core::AstNodeId& astNodeId, const InfoQuery& infoQuery)
{
  return transaction([&,this]()
  {
    using namespace model;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto defNode = query.loadDefinitionOrDeclaration(astNode);

    auto handler = SymbolHandler::getHandler(db, defNode.symbolType);

    return handler->getSubInfoTree(defNode, infoQuery);
  });
}

std::vector<InfoNode> JavaServiceHelper::getInfoTreeForFile(
  const core::FileId& fileId)
{
  return transaction([&,this]()
    {
      JavaFileHandler fHandler(db);
      return fHandler.getInfoTreeForFile(stoull(fileId.fid));
    });
}

std::vector<InfoNode> JavaServiceHelper::getSubInfoTreeForFile(
  const core::FileId& fileId, const InfoQuery& infoQuery)
{
  return transaction([&,this]()
    {
      JavaFileHandler fHandler(db);
      return fHandler.getSubInfoTreeForFile(stoull(fileId.fid), infoQuery);
    });
}

model::SymbolType JavaServiceHelper::getNodeKind(const core::AstNodeId& astNodeId)
{
  return transaction([&, this]() {
    return query.loadAstNode(astNodeId.astNodeId).symbolType;
  });
}

std::vector<model::JavaAstNode> JavaServiceHelper::unique(
  odb::result<model::JavaAstNode> original)
{
  std::vector<model::JavaAstNode> result;

  const auto bigNumber = 1024;
  result.reserve(bigNumber);
  result.insert(result.end(), original.begin(), original.end());

  std::sort(result.begin(), result.end(),
    [](const model::JavaAstNode& lhs, const model::JavaAstNode& rhs)
    {
      model::Range lhs_range = JavaOdbQuery::getRange(lhs);
      model::Range rhs_range = JavaOdbQuery::getRange(rhs);

      auto lFileId = lhs.file ? lhs.file.object_id() : -1;
      auto rFileId = rhs.file ? rhs.file.object_id() : -1;

      if(lFileId != rFileId)
        return lFileId < rFileId;
/*
      if(lhs_range.start != rhs_range.start)
        return lhs_range.start < rhs_range.start;

      return lhs_range.end < rhs_range.end;
*/
      return lhs_range < rhs_range;
    });

  auto newEnd = std::unique(result.begin(), result.end(),
    [](const model::JavaAstNode& lhs, const model::JavaAstNode& rhs)
    {
      auto lFileId = lhs.file ? lhs.file.object_id() : -1;
      auto rFileId = rhs.file ? rhs.file.object_id() : -1;
      return lFileId == rFileId &&
             lhs.astValue == rhs.astValue &&
             JavaOdbQuery::getRange(lhs) == JavaOdbQuery::getRange(rhs);
    });

  result.resize(std::distance(result.begin(), newEnd));

  return result;
}

std::string JavaServiceHelper::getDiagram(const core::AstNodeId& astNodeId,
  const core::DiagramId::type diagramType)
{
  switch (diagramType)
  {
    case core::DiagramId::FUNCTION_CALL: return getFunctionCallDiagram(astNodeId);
    case core::DiagramId::CLASS:         return getClassDiagram(astNodeId);
    case core::DiagramId::FULL_CLASS:    return getFullClassDiagram(astNodeId);
    default:            return std::string();
  }
}

std::string JavaServiceHelper::getLegend(const core::DiagramId::type diagramType)
{
  switch (diagramType)
  {
    case core::DiagramId::FUNCTION_CALL: return diagram::FunctionCallDiagramHelper::getLegend();
    case core::DiagramId::CLASS:         return diagram::ClassDiagramHelper::getLegend();
    case core::DiagramId::FULL_CLASS:    return diagram::FullClassDiagramHelper::getLegend();
    default: return std::string();
  }
}

std::string JavaServiceHelper::getDiagram(const core::AstNodeId& astNodeId1,
  const core::AstNodeId& astNodeId2)
{
  using namespace model;
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  std::map<std::string, util::Graph::Subgraph> files;

  model::JavaAstNode astNodeFrom;
  model::JavaAstNode astNodeTo;
  transaction([&, this]() {
    auto aFrom = query.loadAstNode(astNodeId1.astNodeId);
    auto aTo = query.loadAstNode(astNodeId2.astNodeId);
    astNodeFrom = query.loadDefinition(aFrom);
    astNodeTo   = query.loadDefinitionOrDeclaration(aTo);
  });

  SLog(util::ERROR) << "From: " << astNodeFrom.id
                       << "To: "   << astNodeTo.id
                       << std::endl;

  //--- Helper function ---//

  auto addFileSubgraphIfNeeded =
    [&files, &graph, this](const model::JavaAstNode& astNode) {
      std::string filename;

      auto definition = query.loadDefinitionOrDeclaration(astNode);

      if (definition.file)
        filename = definition.file.load()->path;
      else
        return filename;

      if (files.find(filename) == files.end())
      {
        util::Graph::Subgraph fileGraph = graph.addSubgraph("cluster_" + filename);
        graph.setAttribute(fileGraph, "label", filename);
        files[filename] = fileGraph;
      }

      return filename;
    };

  auto getNodeLabel = [this](const model::JavaAstNode& astNode) {
    return astNode.astValue;//query.queryEntityByHash<model::JavaFunction>(astNode.mangledNameHash).qualifiedName;
  };

  transaction([&, this]() {
    typedef std::map<model::JavaAstNode,
            std::set<std::pair<model::JavaAstNode, bool>>> ParentMap;

    std::queue<model::JavaAstNode> q;
    q.push(astNodeFrom);

    std::set<model::JavaAstNode> visited;
    visited.insert(astNodeFrom);

    ParentMap parent;

    // TODO: this and the next loop is quite similar. Optimisation?
    while (!q.empty())
    {
      auto current = q.front();
      q.pop();

      for (const auto& calledAstNode : db->query<model::JavaAstNode>(
        JavaOdbQuery::queryCallsInAstNode(current)))
      {
        model::JavaAstNode definition;
        try { // TODO: a virtuális függvények miatt van itt kivételkezelés
          definition = query.loadDefinitionOrDeclaration(calledAstNode);
        } catch (...) { continue; }

        auto it = visited.find(definition);
        if (it == visited.end())
        {
          visited.insert(it, definition);
          q.push(definition);
        }

        if (calledAstNode.astType == model::AstType::VirtualCall)
        {
          parent[definition].insert({current, true});
/*
          auto overriders = query.getTransitiveClosureOfRel(
            JavaRelation::Override, calledAstNode.mangledNameHash);

          for (auto overrider : overriders)
          {
            auto func = query.queryEntityByHash<JavaFunction>(overrider);

            auto ast = *func.astNodePtr.load();

            parent[ast].insert({current, true});

            auto it = visited.find(ast);
            if (it == visited.end())
            {
              visited.insert(it, ast);
              q.push(ast);
            }
          }
*/
        }
        else
        {
          parent[definition].insert({current, false});
        }
      }
    }

    visited.clear();
    visited.insert(astNodeTo);
    q.push(astNodeTo);

    std::map<uint64_t, util::Graph::Node> nodeMap;

    util::Graph::Node nodeTo
      = graph.addNode(files[addFileSubgraphIfNeeded(astNodeTo)]);
    graph.setAttribute(nodeTo, "id", std::to_string(astNodeTo.id));
    graph.setAttribute(nodeTo, "label", getNodeLabel(astNodeTo));
    graph.setAttribute(nodeTo, "style", "filled");
    graph.setAttribute(nodeTo, "fillcolor", "gold");

    nodeMap[astNodeTo.id] = nodeTo;

    while (!q.empty())
    {
      auto current = q.front();
      q.pop();

      util::Graph::Node currentNode = nodeMap[current.id];

      for (const auto& par : parent[current])
      {
        auto it = visited.find(par.first);
        if (it == visited.end())
        {
          util::Graph::Node parentNode = graph.addNode(files[addFileSubgraphIfNeeded(par.first)]);
          graph.setAttribute(parentNode, "id", std::to_string(par.first.id));
          graph.setAttribute(parentNode, "label", getNodeLabel(par.first));

          visited.insert(it, par.first);
          q.push(par.first);
          nodeMap[par.first.id] = parentNode;
        }

        auto edge = graph.addEdge(nodeMap[par.first.id], currentNode);

        if (par.second)
        {
          graph.setAttribute(edge, "style", "dashed");
        }
      }
    }

    auto fromNodeIt = nodeMap.find(astNodeFrom.id);
    if (fromNodeIt != nodeMap.end())
    {
      graph.setAttribute(fromNodeIt->second, "style", "filled");
      graph.setAttribute(fromNodeIt->second, "fillcolor", "gold");
    }
  });

  return graph.output(util::Graph::SVG);
}

std::string JavaServiceHelper::getFileDiagram(const core::FileId& fileId,
  const core::DiagramId::type diagramType)
{
  switch (diagramType)
  {
    case core::DiagramId::INCLUDE_DEPENDENCY: return getIncludeDependencyDiagram(fileId);
//    case core::DiagramId::BUILD: return getBuildDiagramLocal(fileId);
//    case core::DiagramId::COMPONENT: return getBuildDiagramComponent(fileId);
    default: return std::string();
  }
}

std::vector<AstNodeInfo> JavaServiceHelper::getFunctionCalls(
  const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto defNode = query.loadDefinition(astNode);
    for (auto call : db->query<model::JavaAstNode>(
                          JavaOdbQuery::queryCallsInAstNode(defNode)))
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });
}

std::string JavaServiceHelper::getFullClassDiagram(
  const core::AstNodeId& astNodeId)
{
  diagram::FullClassDiagramHelper fcd(db, query);

  transaction([&, this](){
    fcd.build(astNodeId.astNodeId);
  });

  return fcd.getOutput();
}

std::string JavaServiceHelper::getFunctionCallDiagram(
  const core::AstNodeId& astNodeId)
{
  diagram::FunctionCallDiagramHelper fcd(db, query);

  transaction([&, this]()
  {
    fcd.build(astNodeId.astNodeId);
  });

  return fcd.getOutput();
}

std::vector<AstNodeInfo> JavaServiceHelper::getCallerFunctions(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    for (auto call : db->query<model::JavaAstNode>(JavaOdbQuery::queryCallers(astNode) && JavaOdbQuery::astFileId(stoull(fileId.fid))))
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });
}

std::vector<AstNodeInfo> JavaServiceHelper::getFunctionAssigns(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    for (auto call : db->query<model::JavaAstNode>(
      JavaOdbQuery::queryReads(astNode) &&
      JavaOdbQuery::astFileId(stoull(fileId.fid))))
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });
}

core::RangedHitCountResult
JavaServiceHelper::makeRangedHitCountResult(
  odb::result<model::AstCountGroupByFilesJava> resultSet,
  const int pageSize, const int pageNo)
{
  core::RangedHitCountResult result;

  int totalCnt = 0;

  auto resultIter = resultSet.begin();
  while (resultIter != resultSet.end())
  {
    if (totalCnt >= (pageNo * pageSize)
      && totalCnt < ((pageNo + 1) * pageSize))
    {
      auto element = *resultIter;
      if (element.file != 0)
      {
        core::HitCountResult hitcount;

        model::File file;
        db->load<model::File>(element.file, file);

        hitcount.finfo.file.fid = std::to_string(file.id);
        hitcount.finfo.name = file.filename;
        hitcount.finfo.path = file.path;

        hitcount.matchingLines = element.count;

        result.results.push_back(hitcount);
      }
    }

    ++totalCnt, ++resultIter;
  }

  result.firstFileIndex = (pageNo * pageSize) + 1;
  result.totalFiles = totalCnt;

  return result;
}

core::RangedHitCountResult JavaServiceHelper::getReferencesPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo)
{
  typedef odb::query<model::AstCountGroupByFilesJava> ACQuery;

  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto resultSet = db->query<model::AstCountGroupByFilesJava>(
      ACQuery::JavaAstNode::mangledNameHash == astNode.mangledNameHash
    );

    return makeRangedHitCountResult(resultSet, pageSize, pageNo);
  });
}

core::RangedHitCountResult JavaServiceHelper::getCallerFunctionsPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo)
{
  typedef odb::query<model::AstCountGroupByFilesJava> ACQuery;

  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto resultSet = db->query<model::AstCountGroupByFilesJava>(
      ACQuery::JavaAstNode::mangledNameHash == astNode.mangledNameHash &&
      (
        ACQuery::JavaAstNode::astType == model::AstType::Usage
        ||
        ACQuery::JavaAstNode::astType == model::AstType::VirtualCall
	  )
    );

    return makeRangedHitCountResult(resultSet, pageSize, pageNo);
  });
}

std::vector<AstNodeInfo> JavaServiceHelper::getReferences(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    for (auto ref : db->query<model::JavaAstNode>(
                          JavaOdbQuery::astMangledNameHash(astNode.mangledNameHash) &&
                          JavaOdbQuery::astFileId(stoull(fileId.fid))) )
    {
      const auto& info = createAstNodeInfo(ref);

      ret.push_back(info);
    }

    return ret;
  });
}

} // language
} // service
} // cc

