/*
 * $Id$
 *
 *  Created on: May 6, 2013
 *      Author: ezoltbo
 */
#include <iostream>
#include <queue>
#include <utility>
#include <cassert>
#include <regex>
#include <sstream>
#include <set>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/tracer.hxx>

#include <util/streamlog.h>
#include "util/standarderrorlogstrategy.h"

#include <util/graph.h>
#include <util/graphbuilder.h>
#include <util/util.h>
#include <langservicelib/utils.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/cxx/cppfunction.h>
#include <model/cxx/cppfunction-odb.hxx>
#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>
#include <model/cxx/cpptypedef.h>
#include <model/cxx/cpptypedef-odb.hxx>
#include <model/cxx/cppinheritance.h>
#include <model/cxx/cppinheritance-odb.hxx>
#include <model/cxx/cppfriendship.h>
#include <model/cxx/cppfriendship-odb.hxx>
#include <model/cxx/cppmacroexpansion.h>
#include <model/cxx/cppmacroexpansion-odb.hxx>
#include <model/cxx/cppheaderinclusion.h>
#include <model/cxx/cppheaderinclusion-odb.hxx>
#include <model/cxx/cppimplicit.h>
#include <model/cxx/cppimplicit-odb.hxx>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildtarget.h>
#include <model/buildtarget-odb.hxx>
#include <model/buildsource.h>
#include <model/buildsource-odb.hxx>

#include "cppservicehelper/cppservicehelper.h"
#include "service/cppservice/src/cppservice.h"

#include "cppservicehelper/symbolhandler.h"
#include "utils.h"
#include <util/filesystem.h>
#include <util/diagram/legendbuilder.h>

#include "slicer/slicerhelper.h"

#include "diagram/diagram.h"


namespace cc
{ 
namespace service
{  
namespace language
{

CppServiceHelper::CppServiceHelper(const std::shared_ptr<odb::database>& db_)
  : db(db_), transaction(db_), query(db_)
{
}

AstNodeInfo CppServiceHelper::getAstNodeInfoByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  return transaction([&, this]()
  {
    auto min = selectProperAstNode(queryAstNodesByPosition(fpos, filters));

    return createAstNodeInfo(min);
  });
}

model::CppAstNode CppServiceHelper::selectProperAstNode(odb::result<model::CppAstNode> nodes)
{
  model::Range minRange(model::Position(0,0), model::Position());
  model::CppAstNode min;
  
  // TODO: handle clickable constructors better
  model::Range minCtorRange = minRange;
  model::CppAstNode minCtor;

  for (const model::CppAstNode& actual : nodes)
  {
    const model::Range& rng = actual.location.range;

    //TODO: remove ugly hack and use
    // CppAstNode::visibleInSourceCode when it will be available
    if (actual.symbolType == model::CppAstNode::SymbolType::Macro)
    {
      return actual;
    }
    
    if (isClickable(actual))
    {
      SLog() << "$$$$$ " << actual.astValue << " " << actual.id;
      
      if (rng < minRange || !isClickable(min))
      {
        min = actual;
        minRange = rng;
      }
      
      if (rng < minCtorRange && actual.astValue.find("construct call") == 0)
      {
        minCtor = actual;
        minCtorRange = rng;
      }
    }   
  }
  
  if (model::isTypeLocation(min.astType) && !minCtor.astValue.empty())
  {
    // If this is a type location and there is a constructor nearby then we
    // should jump to the constructor.
    
    return query.queryEntityByHash<model::CppAstNode>(
              util::fnvHash(minCtor.mangledName));
  }

  return min;
}

InfoBox CppServiceHelper::getInfoBox(const core::AstNodeId& astNodeId)
{
  return transaction([&, this]{
    InfoBox ret;

    auto sHandler = SymbolHandler::getHandler(db, getNodeKind(astNodeId));
    model::CppAstNode cppAstNode
      = *db->load<model::CppAstNode>(std::stoul(astNodeId.astNodeId));

    bool isImplicit = false;
    try
    {
      const model::CppImplicit & imp =
        query.queryEntityByHash<model::CppImplicit>(
        cppAstNode.mangledNameHash);
      isImplicit = true;
    }
    catch(...)
    {
      // absorb.
    }
    
    if (!isImplicit)
      ret.information = sHandler->getInfoBoxText(cppAstNode);

    if (cppAstNode.mangledNameHash)
      ret.documentation
        = getDocCommentInTransaction(cppAstNode.mangledNameHash);

    if (ret.documentation.empty())
    {
      ret.documentation = "<b>Description:</b><br/>";
      switch (cppAstNode.symbolType)
      {
        case model::CppAstNode::SymbolType::Function:
          {
            auto cppFunction = query.queryEntityByHash<model::CppFunction>(
              cppAstNode.mangledNameHash);
            ret.documentation += cppFunction.name;
          }
          break;

        case model::CppAstNode::SymbolType::Variable:
          {
            auto cppVariable = query.queryEntityByHash<model::CppVariable>(
              cppAstNode.mangledNameHash);
            ret.documentation += cppVariable.qualifiedType
                              + ' '
                              +  cppVariable.qualifiedName;
          }
          break;

        case model::CppAstNode::SymbolType::Type:
          {
            auto cppType = query.queryEntityByHash<model::CppType>(
              cppAstNode.mangledNameHash);
            ret.documentation += cppType.name;
          }
          break;

        default:
          ret.documentation.clear();
          break;
      }
    }

    switch (cppAstNode.symbolType)
    {
      case model::CppAstNode::SymbolType::Function:
        {
          try
          {
            const model::CppImplicit & imp =
              query.queryEntityByHash<model::CppImplicit>(
              cppAstNode.mangledNameHash);

            std::ostringstream implss;
            implss << "/* ----------------------------------------------------------------" <<
              "\n * Function(s) below are compiler-generated definitions retrieved from build information." <<
              "\n * Please note that the list of functions generated can differ across different template specializations of the same type." <<
              "\n */\n\n";
            
            implss << imp.code;
            ret.information = implss.str();
          }
          catch(...)
          {
            // queryEntityByHash throws an exception if not found
          }
        }
        break;

      case model::CppAstNode::SymbolType::Type:
        {
          std::ostringstream implss;

          typedef odb::query<model::CppImplicit> QImplicit;

          bool need_line = true;
          for(const model::CppImplicit & imp : db->query<model::CppImplicit>(
            QImplicit::typeHash==cppAstNode.mangledNameHash))
          {
            if(need_line)
            {
              implss << "\n\n/* ----------------------------------------------------------------" <<
                "\n * Functions below are compiler-generated definitions retrieved from build information." <<
                "\n * Please note that the list of functions generated can differ across different template specializations of the same type." <<
                "\n */\n" <<
                "\nnamespace __cc_show_compiler_generated_\n{\n";
              need_line = false;
            }

            implss << "\n";
            implss << imp.code;
            implss << "\n";
          }

          std::ostringstream ss;
          ss << ret.information;
          
          if(!need_line)
          {
            implss << "\n}\n\n/* ---------------------------------------------------------------- */";
            ss << implss.str();
          }
          
          ret.information = ss.str();
          
          // Sorting members
          auto sortMember = [this](const model::CppMemberType& lhs, const model::CppMemberType& rhs)
            {
              auto left  = std::make_tuple(lhs.isStatic, lhs.memberTypeHash, lhs.visibility);
              auto right = std::make_tuple(rhs.isStatic, rhs.memberTypeHash, rhs.visibility);
              if (left < right)
                return true;
              else if (left == right)
              {
                auto lhsEntityResult = query.queryEntityByHash(lhs.typeHash);
                auto rhsEntityResult = query.queryEntityByHash(rhs.typeHash);
                
                auto& lhsEntity = *lhsEntityResult.begin();
                auto& rhsEntity = *rhsEntityResult.begin();
                
                return lhsEntity.name < rhsEntity.name;
              }
              return false;
            };
          
          // Print info group box
          auto printInfoBoxGroup = [this](const std::vector<model::CppMemberType>& functions,
            std::string groupHeaderLabel)
          {
            std::string documentation = "";
            std::string prevGroupHeader; // Previous group header      
            for (const auto& memFun : functions)
            {
              memFun.memberAstNode.load();

              std::string docComment = 
                getDocCommentInTransaction(memFun.memberAstNode->mangledNameHash);

              if (!docComment.empty()) // Doc comment is not empty
              {
                std::ostringstream ss;

                std::string s = "Description"; // Remove this from doc comment
                std::string::size_type i = docComment.find(s);

                if (i != std::string::npos)
                   docComment.erase(i, s.length());

                std::string actualGroupHeader = ""; // Actual group header
                std::string visibility = visibilityToString(memFun.visibility);
                visibility[0] = std::toupper(visibility[0]);
                if(memFun.isStatic)
                    actualGroupHeader = "Static ";
                actualGroupHeader += visibility;

                if(prevGroupHeader == "" || prevGroupHeader != actualGroupHeader)
                {
                  ss << "<div class=\"group-header\"><dl><dt><b>";      
                  ss << actualGroupHeader << groupHeaderLabel;
                  ss << "</dt></dl></b></div>";
                }

                ss << "<div class=\"group\">";
                ss << docComment;
                ss << "</div>";

                documentation += ss.str();

                prevGroupHeader = actualGroupHeader;
              }
            }
            return documentation;
          };
          
          auto cppType = query.queryEntityByHash<model::CppType>(
            cppAstNode.mangledNameHash);
           
          // Get members
          auto fields_result = query.queryMembersByType(cppType,
            model::CppMemberType::Kind::Field);

          std::vector<model::CppMemberType> fields = { fields_result.begin(),
            fields_result.end() };

          std::sort(fields.begin(), fields.end(), sortMember);
          
          ret.documentation += printInfoBoxGroup(fields, " Members:");
          
          // Get member functions    
          auto funtions_result = query.queryMembersByType(cppType,
            model::CppMemberType::Kind::Method);

          std::vector<model::CppMemberType> functions = { funtions_result.begin(),
            funtions_result.end() };   
      
          std::sort(functions.begin(), functions.end(), sortMember);
            
          ret.documentation += printInfoBoxGroup(functions, " Member Functions:");          
        }
        break;
        
      default:
        break;
    }

    return ret;
  });
}

InfoBox CppServiceHelper::getInfoBoxByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  return transaction([&, this]()
  {
    try
    {
      InfoBox ret;

      auto min = selectProperAstNode(queryAstNodesByPosition(fpos, filters));
      auto sHandler = SymbolHandler::getHandler(db, min.symbolType);

      ret.information = sHandler->getInfoBoxText(min);
      ret.fileType = core::FileType::CxxSource;
      if (min.mangledNameHash) {
        ret.documentation = getDocCommentInTransaction(min.mangledNameHash);
      }

      // TODO: This has to be refactored!
      if (ret.documentation.empty())
      {
        ret.documentation = "<b>Description:</b><br/>";
        switch (min.symbolType)
        {
          case model::CppAstNode::SymbolType::Function:
            {
              auto cppFunction = query.queryEntityByHash<model::CppFunction>(
                min.mangledNameHash);
              ret.documentation += cppFunction.name;
            }
            break;

          case model::CppAstNode::SymbolType::Variable:
            {
              auto cppVariable = query.queryEntityByHash<model::CppVariable>(
                min.mangledNameHash);
              ret.documentation += cppVariable.qualifiedType
                                + ' '
                                +  cppVariable.qualifiedName;
            }
            break;

          case model::CppAstNode::SymbolType::Type:
            {
              auto cppType = query.queryEntityByHash<model::CppVariable>(
                min.mangledNameHash);
              ret.documentation += cppType.name;
            }
            break;
          case model::CppAstNode::SymbolType::Enum:
          case model::CppAstNode::SymbolType::EnumConstant:
          case model::CppAstNode::SymbolType::Typedef:
          case model::CppAstNode::SymbolType::FunctionPtr:
          case model::CppAstNode::SymbolType::Macro:
            {
              auto defNode
                = query.loadDefinitionOrDeclaration(min)[0];
              
              const auto& fileContent =
                defNode.location.file.load()->content.load()->content;
              
              ret.documentation += textRange(fileContent, defNode.location.range);
            }
            break; 
          default:
            ret.documentation.clear();
            break;
        }
      }

      switch (min.symbolType)
      {
        case model::CppAstNode::SymbolType::Function:
          {
            try
            {
              const model::CppImplicit & imp =
                query.queryEntityByHash<model::CppImplicit>(
                min.mangledNameHash);

              ret.information += "\n//----------------------------------------------------------------";
              ret.information += "\n\n";
              ret.information += imp.code;
              ret.information += "\n";
            }
            catch(...)
            {
              // queryEntityByHash throws an exception if not found
            }
          }
          break;

        case model::CppAstNode::SymbolType::Type:
          {
            std::ostringstream ss;

            typedef odb::query<model::CppImplicit> QImplicit;

            bool need_line = true;
            for(const model::CppImplicit & imp : db->query<model::CppImplicit>(
              QImplicit::typeHash==min.mangledNameHash))
            {
              if(need_line)
              {
                ss << ret.information << "\n//----------------------------------------------------------------";
                need_line = false;
              }

              ss << "\n\n";
              ss << imp.code;
              ss << "\n";
            }

            if(!need_line)
            {
              ret.information = ss.str();
            }
          }
          break;

        default:
          break;
      }

      return ret;
    }
    catch (const std::exception& ex)
    {
      SLog(util::WARNING)
        << "Exception caught: " << ex.what();
    }

    return InfoBox();
  });
}

odb::result<model::CppAstNode> CppServiceHelper::queryAstNodesByPosition(
  const core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  model::Position position;
  position.line = fpos.pos.line;
  position.column = fpos.pos.column;

  SLog(util::DEBUG) << "Start searching";

  auto result = db->query<model::CppAstNode>(
    CppOdbQuery::astFileId(stoull(fpos.file.fid)) && CppOdbQuery::astContains(position));

  if (result.empty())
  {
    throw core::InvalidPosition();
  }
  return result;
}

std::string CppServiceHelper::getSourceCode(
  const core::AstNodeId& astNodeId)
{
  std::string fileContent;
  model::Range range;
  transaction([&, this]() {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    fileContent = astNode.location.file.load()->content.load()->content;
    range = astNode.location.range;
  });

  return textRange(fileContent, range);
}

//must be in an odb transaction to call this
std::string CppServiceHelper::getDocCommentInTransaction(
  const model::HashType mangledNameHash_)
{
  std::string ret;

  model::Range range;
  auto docComments = query.loadDocComments(mangledNameHash_);

  //TODO this is not optimal
  for (const model::DocComment& dc: docComments)
  {
    ret += dc.contentHTML;
  }

  return ret;
}

std::string CppServiceHelper::getDocComment(
  const core::AstNodeId& astNodeId)
{
  throw std::logic_error("not implemented");
}

AstNodeInfo CppServiceHelper::getAstNodeInfo(
const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    return createAstNodeInfo(query.loadAstNode(astNodeId.astNodeId));
  });
}

std::string CppServiceHelper::getClassDiagram(
  const core::AstNodeId& astNodeId)
{
  util::Graph graph;
  graph.setAttribute("rankdir", "BT");

  std::map<std::pair<std::string, bool>, util::Graph::Node> edges;

  model::CppAstNode thisAstNode;
  model::CppType cppType;

  transaction([&, this]() {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    thisAstNode = query.loadDefinition(astNode);
    cppType = query.queryEntityByHash<model::CppType>(thisAstNode.mangledNameHash);
  });

  //--- Helper functions ---//

  auto getNodeLabel
    = [this](const model::CppAstNode& node, const model::CppType& cppType) {
      auto getVisibility = [](const model::Visibility& visibility) {
        switch (visibility)
        {
          case model::Public:    return '+';
          case model::Private:   return '-';
          case model::Protected: return '#';
        }
        return ' ';
      };

      std::string label = '{' + node.astValue + '|';

      for (const model::CppMemberType& member : query.queryMembersByType(
        cppType, model::CppMemberType::Kind::Field))
      {
        label += getVisibility(member.visibility)
                + member.memberAstNode.load()->astValue
                +  "\\l";
      }
      label += '|';
      for (const model::CppMemberType& member : query.queryMembersByType(
        cppType, model::CppMemberType::Kind::Method))
      {
        member.memberAstNode.load();
        label += getVisibility(member.visibility)
                +  member.memberAstNode->astValue + "()"
                +  "\\l";
      }

      label += '}';

      return escapeDot(label);
  };

  auto makeInheritanceNodes
    = [&, this](odb::result<model::CppInheritance> nodes, bool out) { // TODO: nodes should be passed by const reference, but it doesn't have const iterator
      for (const model::CppInheritance& node : nodes)
      {
        model::CppType cppType = query.queryEntityByHash<model::CppType>(
          out ? node.base : node.derived);

        auto otherAstNode = *db->load<model::CppAstNode>(
          cppType.astNodeId.get());

        std::string nodeLabel = getNodeLabel(otherAstNode, cppType);

        std::pair<std::string, bool> p(nodeLabel, out);

        util::Graph::Node otherNode = graph.addNode();
        graph.setAttribute(otherNode, "label", nodeLabel);
        graph.setAttribute(otherNode, "id", std::to_string(otherAstNode.id));
        graph.setAttribute(otherNode, "shape", "record");
        edges[p] = otherNode;
      }
    };

  transaction([&, this]() {
    //--- Node for current class ---//

    util::Graph::Node thisNode = graph.addNode();
    graph.setAttribute(thisNode, "id", std::to_string(thisAstNode.id));
    graph.setAttribute(thisNode, "label", getNodeLabel(thisAstNode, cppType));
    graph.setAttribute(thisNode, "shape", "record");

    //--- Nodes for inheritance classes ---//

    typedef odb::query<model::CppInheritance> QInheritance;

    makeInheritanceNodes(
      db->query<model::CppInheritance>(QInheritance::derived == cppType.mangledNameHash), true);
    makeInheritanceNodes(
      db->query<model::CppInheritance>(QInheritance::base == cppType.mangledNameHash), false);

    //--- Edges ---//

    for (auto it : edges)
    {
      util::Graph::Edge edge = it.first.second
                             ? graph.addEdge(thisNode, it.second)
                             : graph.addEdge(it.second, thisNode);
      graph.setAttribute(edge, "arrowhead", "empty");
    }
  });

  return graph.output(util::Graph::SVG);
}

std::vector<AstNodeInfo> CppServiceHelper::getDefinition(
const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    
    std::vector<model::CppAstNode> defs
      = query.loadDefinitionOrDeclaration(astNode);
    std::vector<AstNodeInfo> nodeInfos(defs.size());
    
    std::transform(defs.begin(), defs.end(), nodeInfos.begin(), createAstNodeInfo);
    
    return nodeInfos;
  });
}

std::vector<core::AstNodeId> CppServiceHelper::getTypeDefinitions(
  const std::string& path)
{
  return transaction([&, this]()
  {

    typedef odb::query<model::FileIdView> FQ;

    auto fileIdResult = db->query<model::FileIdView>(FQ::path.like(path + '%'));

    std::vector<model::FileId> ids;

    std::transform(fileIdResult.begin(), fileIdResult.end(), std::back_inserter(ids),
      [](const model::FileIdView& fidView) { return fidView.id; });

    typedef odb::query<model::CppAstNodeId> AQ;

    auto astIdResult = db->query<model::CppAstNodeId>(
      AQ::symbolType == model::CppAstNode::SymbolType::Type &&
      AQ::astType == model::CppAstNode::AstType::Definition &&
      AQ::location.file.in_range(ids.begin(), ids.end()));

    std::vector<core::AstNodeId> ret;
    for (auto astId : astIdResult)
    {
      core::AstNodeId coreAstId;

      coreAstId.astNodeId = std::to_string(astId.id);

      ret.push_back(std::move(coreAstId));
    }

    return ret;
  });
}

std::vector<InfoNode> CppServiceHelper::getInfoTree(
  const core::AstNodeId& astNodeId)
{
  return transaction([&,this]()
  {
    using namespace model;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto defNode = query.loadDefinitionOrDeclaration(astNode)[0];

    auto handler = SymbolHandler::getHandler(db, defNode.symbolType);

    return handler->getInfoTree(defNode);
  });
}

std::vector<InfoNode> CppServiceHelper::getSubInfoTree(
  const core::AstNodeId& astNodeId, const InfoQuery& infoQuery)
{
  return transaction([&,this]()
  {
    using namespace model;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto defNode = query.loadDefinitionOrDeclaration(astNode)[0];

    auto handler = SymbolHandler::getHandler(db, defNode.symbolType);

    return handler->getSubInfoTree(defNode, infoQuery);
  });
}

std::vector<InfoNode> CppServiceHelper::getInfoTreeForFile(
  const core::FileId& fileId)
{
  return transaction([&,this]()
    {
      CppFileHandler fHandler(db);

      model::FileId fid = stoull(fileId.fid);

      return fHandler.getInfoTreeForFile(fid);
    });
}

std::vector<InfoNode> CppServiceHelper::getSubInfoTreeForFile(
  const core::FileId& fileId, const InfoQuery& infoQuery)
{
  return transaction([&,this]()
    {
      CppFileHandler fHandler(db);

      model::FileId fid = stoull(fileId.fid);

      return fHandler.getSubInfoTreeForFile(fid, infoQuery);
    });
}

model::CppAstNode::SymbolType CppServiceHelper::getNodeKind(const core::AstNodeId& astNodeId)
{
  return transaction([&, this]() {
    return query.loadAstNode(astNodeId.astNodeId).symbolType;
  });
}

std::vector<model::CppAstNode> CppServiceHelper::unique(
  odb::result<model::CppAstNode> original)
{
  std::vector<model::CppAstNode> result;

  const auto bigNumber = 1024;
  result.reserve(bigNumber);
  result.insert(result.end(), original.begin(), original.end());

  std::sort(result.begin(), result.end(),
    [](const model::CppAstNode& lhs, const model::CppAstNode& rhs)
    {
      const auto& leftStart = lhs.location.range.start;
      const auto& leftEnd = lhs.location.range.end;

      const auto& rightStart = rhs.location.range.start;
      const auto& rightEnd = rhs.location.range.end;

      auto lFileId = lhs.location.file ? lhs.location.file.object_id() : -1;
      auto rFileId = rhs.location.file ? rhs.location.file.object_id() : -1;

      if (lFileId != rFileId)
        return lFileId < rFileId;

      if (leftStart != rightStart)
        return leftStart < rightStart;

      return leftEnd < rightEnd;
    });

  auto newEnd = std::unique(result.begin(), result.end(),
    [](const model::CppAstNode& lhs, const model::CppAstNode& rhs)
    {
      auto lFileId = lhs.location.file ? lhs.location.file.object_id() : -1;
      auto rFileId = rhs.location.file ? rhs.location.file.object_id() : -1;
      return lFileId == rFileId &&
             lhs.astValue == rhs.astValue &&
             lhs.location.range == rhs.location.range;
    });

  result.resize(std::distance(result.begin(), newEnd));

  return result;
}

std::string CppServiceHelper::getDiagram(const core::AstNodeId& astNodeId,
  const core::DiagramId::type diagramId)
{
  switch (diagramId)
  {
    case core::DiagramId::FUNCTION_CALL:
      return getFunctionCallDiagram(astNodeId);
    
    case core::DiagramId::CLASS:
      return getClassDiagram(astNodeId);
    
    case core::DiagramId::FULL_CLASS:
      return getFullClassDiagram({astNodeId});
    
    case core::DiagramId::CLASS_COLLABORATION:
      return getClassCollaborationDiagram(astNodeId);
      
    case core::DiagramId::POINTER_ANALYSIS: 
    {      
      return transaction([&, this]() {
        auto sHandler = SymbolHandler::getHandler(db, model::CppAstNode::SymbolType::Variable);
        auto node  = query.loadAstNode(astNodeId.astNodeId);
        return sHandler->getPointerAnalysisDiagram(node);
      });    
    }
    default:
      return std::string();
  }
}

std::string CppServiceHelper::getLegend(const core::DiagramId::type diagramType)
{
  // TODO: The implementation of the case statements should be placed elsewhere.
  // The javaservicehelper uses a totally different design, which supports
  // the "getLegend()" function for all diagram types. This can not be done
  // with this design.

  switch(diagramType)
  {
  case core::DiagramId::FUNCTION_CALL:
    {
      util::diagram::LegendBuilder builder;
      builder.addNode("center function", { {"style", "filled"},
        {"fillcolor", "gold"} });
      builder.addNode("global function", { {"shape", "star"} });
      builder.addNode("static called", { {"style", "filled"},
        {"fillcolor", "lightblue"} });
      builder.addNode("static caller", { {"style", "filled"},
        {"fillcolor", "coral"} });
      builder.addNode("overridable function", { {"shape", "diamond"},
        {"style", "filled"}, {"fillcolor", "cyan"} });
      builder.addNode("final function", { {"shape", "ellipse"},
        {"style", "filled"}, {"fillcolor", "lightblue"} });
      builder.addEdge("called", { {"color", "blue"} });
      builder.addEdge("caller", { {"color", "red"} });
      builder.addEdge("overrider", { {"style", "dashed"} });
      return builder.getOutput();
    }
    break;

  case core::DiagramId::CLASS:
    {
      util::diagram::LegendBuilder builder;
      builder.addNode("class", { {"shape", "record"} });
      builder.addEdge("inheritance", { {"arrowhead", "empty"} });
      return builder.getOutput();
    }
    break;

  case core::DiagramId::DIR_FULL_CLASS:
  case core::DiagramId::FULL_CLASS:
    {
      util::diagram::LegendBuilder builder;
      builder.addNode("current class", { {"shape", "box"},
        {"style", "filled"}, {"fillcolor", "gold"} });
      builder.addNode("class", { {"shape", "box"} });
      builder.addEdge("inheritance", { {"arrowhead", "empty"}, {"color", "red"} });
      builder.addEdge("aggregation", { {"arrowhead", "diamond"} });
      return builder.getOutput();
    }
    break;  
  case core::DiagramId::CLASS_COLLABORATION:
    {
      util::diagram::LegendBuilder builder;
      builder.addNode("current class", { {"shape", "box"},
        {"style", "filled"}, {"fillcolor", "gold"} });
      builder.addNode("class", { {"shape", "box"} });
      builder.addNode("class for which not all inheritance/containment relations are shown", { {"shape", "box"}, { "color", "red" } });
      builder.addEdge("public inheritance",
        { {"arrowhead", "normal"}, {"color", "darkblue"} });
      builder.addEdge("protected inheritance",
        { {"arrowhead", "normal"}, {"color", "darkgreen"} });
      builder.addEdge("private inheritance",
        { {"arrowhead", "normal"}, {"color", "darkred"} });
      builder.addEdge("contained or used class",
        { {"arrowhead", "normal"}, {"color", "mediumpurple"}, {"style", "dashed"} });
      return builder.getOutput();
    }
    break;
  case core::DiagramId::INTERFACE:
  case core::DiagramId::COMPONENTS_USED:
  case core::DiagramId::COMPONENT_USERS:
    {
      std::vector<std::pair<std::string, std::string>> sourceStyle= {{"label","source.cc"},{"shape", "box"},{"style", "filled"}, {"fillcolor", "#afcbe4"}}; 
      std::vector<std::pair<std::string, std::string>> intfStyle= {{"label","interface.hh"}, {"shape", "ellipse"},{"style", "filled"}, {"fillcolor", "#e4afaf"}};
      std::vector<std::pair<std::string, std::string>> objectStyle= { {"shape", "folder"},{"label","object.o"},{"style", "filled"}, {"fillcolor", "#afe4bf"} };
      std::vector<std::pair<std::string, std::string>> provStyle= { {"label","provides"},{"color", "red"}};
      std::vector<std::pair<std::string, std::string>> usesStyle= { {"label","uses"},{"color", "black"}};
      std::vector<std::pair<std::string, std::string>> containsStyle= { {"label","contains"},{"color", "blue"}};

      
      std::string title="";
      if (diagramType==core::DiagramId::INTERFACE)
        title="This diagram describes the relationship between C/C++ source and header files and objects.";
      else
        if  (diagramType==core::DiagramId::COMPONENTS_USED)
          title="This diagram shows headers and source files used by a selected source or header.";
        if  (diagramType==core::DiagramId::COMPONENT_USERS)
          title="This diagram shows the users of a selected header or a cpp source file based on the symbols declared and defined.";
      
      util::diagram::LegendBuilder builder(title);
      
      {//Adding the description for the implements edge
        
        //this is an invisible node needed to align the subgraphs
        //after each other
        util::Graph::Node hook=util::Graph::Node(); 
        util::Graph::Subgraph sg=builder.addSubgraph("implements",&hook);
        util::Graph* mainGraph=builder.getGraph();
        
        util::Graph::Node provider = mainGraph->addNode(sg);
        builder.setStyle(provider, sourceStyle);
        util::Graph::Node intf = mainGraph->addNode(sg);
        builder.setStyle(intf, intfStyle);
        util::Graph::Edge providesEdge = mainGraph->addEdge(provider, intf);
        builder.setStyle(providesEdge, provStyle);
        
        util::Graph::Edge edge = mainGraph->addEdge(hook, provider);                
        mainGraph->setAttribute(edge, "style", "invis");
        
        util::Graph::Node desc = mainGraph->addNode(sg);
        builder.setStyle(desc, {{"label","source.cc implements at least one function declared in interface.hh"}, {"shape", "none"}});
        util::Graph::Edge desc_edge = mainGraph->addEdge(intf, desc);
        mainGraph->setAttribute(desc_edge, "style", "invis");
        
      }
      
      {//Adding the description for the uses edge
        
        //this is an invisible node needed to align the subgraphs
        //after each other
        util::Graph::Node hook=util::Graph::Node(); 
        util::Graph::Subgraph sg=builder.addSubgraph("implements",&hook);
        util::Graph* mainGraph=builder.getGraph();
        
        util::Graph::Node user = mainGraph->addNode(sg);
        builder.setStyle(user, sourceStyle);
        util::Graph::Node intf = mainGraph->addNode(sg);
        builder.setStyle(intf, intfStyle);
        util::Graph::Edge usesEdge = mainGraph->addEdge(user, intf);
        builder.setStyle(usesEdge, usesStyle);
        
        util::Graph::Edge edge = mainGraph->addEdge(hook, user);                
        mainGraph->setAttribute(edge, "style", "invis");
        
        util::Graph::Node desc = mainGraph->addNode(sg);
        builder.setStyle(desc, {{"label","source.cc uses at least one symbol declared in interface.hh"}, {"shape", "none"}});
        util::Graph::Edge desc_edge = mainGraph->addEdge(intf, desc);
        mainGraph->setAttribute(desc_edge, "style", "invis");
        
      }
      {//Adding the description for the contains
        
        //this is an invisible node needed to align the subgraphs
        //after each other
        util::Graph::Node hook=util::Graph::Node(); 
        util::Graph::Subgraph sg=builder.addSubgraph("implements",&hook);
        util::Graph* mainGraph=builder.getGraph();
        
        util::Graph::Node obj = mainGraph->addNode(sg);
        builder.setStyle(obj, objectStyle);
        util::Graph::Node src = mainGraph->addNode(sg);
        builder.setStyle(src, sourceStyle);
        util::Graph::Edge cntEdge = mainGraph->addEdge(obj, src);
        builder.setStyle(cntEdge, containsStyle);
        
        util::Graph::Edge edge = mainGraph->addEdge(hook, obj);                
        mainGraph->setAttribute(edge, "style", "invis");
        
        util::Graph::Node desc = mainGraph->addNode(sg);
        builder.setStyle(desc, {{"label","source.cc is compiled into object.o"}, {"shape", "none"}});
        util::Graph::Edge desc_edge = mainGraph->addEdge(src, desc);
        mainGraph->setAttribute(desc_edge, "style", "invis");
        
      }
      
      
      
      return builder.getOutput();
  }
      break;
      /*
    case core::DiagramId::COMPONENTS_USED:
      
      break;
    case core::DiagramId::COMPONENT_USERS:
      
      break;
    case core::DiagramId::SUBSYSTEM_DEPENDENCY:
      
      break;
    case core::DiagramId::EXTERNAL_DEPENDENCIES:
      
      break;
    case core::DiagramId::EXTERNAL_USERS:
      
      break;*/
  case core::DiagramId::POINTER_ANALYSIS:
    {
      util::diagram::LegendBuilder builder;
      builder.addNode("current variable", { {"shape", "circle"},
        {"style", "filled"}, {"fillcolor", "gold"} });
        
      builder.addNode("points to variable", { {"shape", "circle"} });
      
      builder.addNode("variable, which can be nullpointer", { {"shape", "Mcircle"} });
            
      builder.addEdge("points to", {});
                
      builder.addEdge("aliases",
      { {"arrowhead", "empty"}, {"style", "dashed"}, {"color", "grey"} });
      return builder.getOutput();
    }
    break;
  default:
    return std::string();
  }
}

std::string CppServiceHelper::getDiagram(
  const core::AstNodeId& astNodeId1,
  const core::AstNodeId& astNodeId2)
{
  core::LongWaitDiagramEx ex;
  ex.what = "Graph generation too long";

  using namespace model;
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  std::map<std::string, util::Graph::Subgraph> files;

  model::CppAstNode astNodeFrom;
  model::CppAstNode astNodeTo;
  transaction([&, this]() {
    auto aFrom  = query.loadAstNode(astNodeId1.astNodeId);
    auto aTo    = query.loadAstNode(astNodeId2.astNodeId);
    astNodeFrom = query.loadDefinition(aFrom);
    astNodeTo   = query.loadDefinitionOrDeclaration(aTo)[0];
  });

  SLog(util::ERROR) << "From: " << astNodeFrom.id
                    << "To: "   << astNodeTo.id
                    << std::endl;

  //--- Helper function ---//

  auto addFileSubgraphIfNeeded =
    [&files, &graph, this](const model::CppAstNode& astNode) {
      std::string filename;

      auto definition = query.loadDefinitionOrDeclaration(astNode)[0];

      if (definition.location.file)
        filename = definition.location.file.load()->path;
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

  auto getNodeLabel = [this](const model::CppAstNode& astNode) {
    return query.queryEntityByHash<model::CppFunction>(astNode.mangledNameHash).qualifiedName;
  };

  transaction([&, this]() {
    typedef std::map<model::CppAstNode,
            std::set<std::pair<model::CppAstNode, bool>>> ParentMap;

    std::queue<model::CppAstNode> q;
    q.push(astNodeFrom);

    std::set<model::CppAstNode> visited;
    visited.insert(astNodeFrom);

    ParentMap parent;

    std::time_t fromTime = std::time(0);
    // TODO: this and the next loop is quite similar. Optimisation?
    while (!q.empty())
    {
      auto current = q.front();
      q.pop();

      for (const auto& calledAstNode : db->query<model::CppAstNode>(
        CppOdbQuery::queryCallsInAstNode(current)))
      {
        model::CppAstNode definition;
        try { // TODO: a virtuális függvények miatt van itt kivételkezelés
          definition = query.loadDefinitionOrDeclaration(calledAstNode)[0];
        } catch (...) { continue; }

        auto it = visited.find(definition);
        if (it == visited.end())
        {
          visited.insert(it, definition);
          q.push(definition);
        }

        if (calledAstNode.astType == model::CppAstNode::AstType::VirtualCall)
        {
          parent[definition].insert({current, true});

          auto overriders = query.getTransitiveClosureOfRel(
            CppRelation::Kind::Override, calledAstNode.mangledNameHash);

          for (auto overrider : overriders)
          {
            auto func = query.queryEntityByHash<CppFunction>(overrider);

            auto ast = *db->load<CppAstNode>(func.astNodeId.get());

            parent[ast].insert({current, true});

            auto it = visited.find(ast);
            if (it == visited.end())
            {
              visited.insert(it, ast);
              q.push(ast);
            }
          }
        }
        else
        {
          parent[definition].insert({current, false});
        }

        if (std::time(0) - fromTime > 5)
          throw ex;
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

std::string CppServiceHelper::getFileDiagram(const core::FileId& fileId,
  const core::DiagramId::type diagramType)
{
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");
  model::FileId fid = std::stoull(fileId.fid);

  switch (diagramType)
  {
    case core::DiagramId::INCLUDE_DEPENDENCY:
      includeDependency(graph, fid, db);
      break;
    case core::DiagramId::INTERFACE:
      interfaceDiagram(graph, fid, db);
      break;
    case core::DiagramId::COMPONENTS_USED:
      usedComponents(graph, fid, db);
      break;
    case core::DiagramId::COMPONENT_USERS:
      componentUsers(graph, fid, db);
      break;
    case core::DiagramId::DIR_FULL_CLASS:
      return getDirectoryFullClassDiagram(fid);
      break;
    case core::DiagramId::SUBSYSTEM_DEPENDENCY:
      subsystemDependency(graph, fid, db);
      break;
    case core::DiagramId::EXTERNAL_DEPENDENCIES:
      externalDependency(graph, fid, db);
      break;
    case core::DiagramId::EXTERNAL_USERS:
      externalUsers(graph, fid, db);
      break;
    default: return std::string();
  }

  return graph.nodeCount() == 0 ? "" : graph.output(util::Graph::SVG);
}

std::string CppServiceHelper::getDirectoryFullClassDiagram(
  const model::FileId& fileId)
{
  typedef odb::query<model::CppAstNodeId> AstIdQuery;
  typedef odb::result<model::CppAstNodeId> AstIdResult;
  
  typedef odb::query<model::FileIdView> FileIdQuery;
  typedef odb::result<model::FileIdView> FileIdResult;
  
  std::vector<core::AstNodeId> nodeIds;
  
  transaction([&, this]{
    
    //--- Get root file from database ---//
    
    model::File rootFile;
    db->find<model::File>(fileId, rootFile);
    
    //--- Get child file ids ---//
    
    FileIdResult files = db->query<model::FileIdView>(
      FileIdQuery::type == model::File::CxxSource &&
      FileIdQuery::parseStatus == model::File::ParseStatus::PSFullyParsed &&
      FileIdQuery::path.like(rootFile.path + '%'));
    
    std::vector<model::FileId> fileIds;
    std::transform(files.begin(), files.end(), std::back_inserter(fileIds),
      [](const model::FileIdView& fidView) { return fidView.id; });
      
    //--- Get type definitions ---//
      
    if(!fileIds.empty())  
    {
      AstIdResult nodes = db->query<model::CppAstNodeId>(
        AstIdQuery::location.file.in_range(fileIds.begin(), fileIds.end()) &&
        AstIdQuery::symbolType == model::CppAstNode::SymbolType::Type &&
        AstIdQuery::astType == model::CppAstNode::AstType::Definition);

      std::transform(nodes.begin(), nodes.end(), std::back_inserter(nodeIds),
        [](const model::CppAstNodeId& nodeId) {
          core::AstNodeId astNodeId;
          astNodeId.astNodeId = std::to_string(nodeId.id);
          return astNodeId;
        });
    }
  });
  
  return getFullClassDiagram(nodeIds);
}

std::vector<AstNodeInfo> CppServiceHelper::getFunctionCalls(
  const core::AstNodeId& astNodeId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto defNode = query.loadDefinition(astNode);
    for (auto call : db->query<model::CppAstNode>(
                          CppOdbQuery::queryCallsInAstNode(defNode)))
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });
}

namespace
{
template <typename T>
struct CompPairFst
{
  bool operator()(const std::pair<T, T>& p, const T& value) const
  {
    return p.first < value;
  }
  
  bool operator()(const T& value, const std::pair<T, T>& p) const
  {
    return value < p.first;
  }
  
  bool operator()(const std::pair<T, T>& p1, const std::pair<T, T>& p2) const
  {
    return p1.first < p2.first;
  }
};

template <typename T>
struct CompPairSnd
{
  bool operator()(const std::pair<T, T>& p, const T& value) const
  {
    return p.second < value;
  }
  
  bool operator()(const T& value, const std::pair<T, T>& p) const
  {
    return value < p.second;
  }
  
  bool operator()(const std::pair<T, T>& p1, const std::pair<T, T>& p2) const
  {
    return p1.second < p2.second;
  }
};
}

std::string CppServiceHelper::getFullClassDiagram(
  const std::vector<core::AstNodeId>& astNodeIds)
{
  //--- Read CppInheritance table into memory ---//
  
  using HashTypePair = std::pair<model::HashType, model::HashType>;
  using Inheritance = std::vector<HashTypePair>;
  
  Inheritance inheritance;
  
  transaction([&inheritance, this]{
    auto all = db->query<model::CppInheritance>();
    std::transform(all.begin(), all.end(), std::back_inserter(inheritance),
      [](const model::CppInheritance& in){
        return std::make_pair(in.derived, in.base);
      });
  });
  
  //--- Create graph ---//
  
  util::Graph graph;
  graph.setAttribute("rankdir", "RL");
  
  using VisitedNodesMap = std::map<model::HashType, util::Graph::Node>;
  VisitedNodesMap visitedNodes;
  
  std::vector<util::Graph::Edge> inhEdges;
  std::vector<util::Graph::Edge> aggrEdges;
  
  // We copy this queue so the contained nodes are not queried again in
  // the next phases where the algorithm goes towards the children in the
  // inheritance or when aggregated nodes are collected.
  std::queue<model::HashType> qOrig;
  
  //--- Adding nodes towards the parents ---//
  
  transaction([&, this]{
    for (const core::AstNodeId& id : astNodeIds)
    {
      model::HashType hash = query.loadAstNode(id.astNodeId).mangledNameHash;
      qOrig.push(hash);
      
      util::Graph::Node node = graph.addNode();
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", "gold");
      
      visitedNodes.insert(std::make_pair(hash, node));
    }
  });
  
  std::sort(inheritance.begin(), inheritance.end(),
    CompPairFst<model::HashType>());
  
  std::queue<model::HashType> q = qOrig;
  
  while (!q.empty())
  {
    model::HashType mNode = q.front();
    q.pop();
    
    auto range = std::equal_range(inheritance.begin(), inheritance.end(), mNode,
      CompPairFst<model::HashType>());
    
    for (auto p = range.first; p != range.second; ++p)
    {
      const auto& targetIt = visitedNodes.find(p->second);
      if (targetIt == visitedNodes.end())
      {
        util::Graph::Node gNode = graph.addNode();
        inhEdges.push_back(graph.addEdge(visitedNodes[p->first], gNode));
        visitedNodes[p->second] = gNode;
        q.push(p->second);
      }
      else
      {
        inhEdges.push_back(
          graph.addEdge(visitedNodes[p->first], targetIt->second));
      }
    }
  }
  
  //--- Adding nodes towards the children ---//
  
  q = qOrig;
  
  std::sort(inheritance.begin(), inheritance.end(),
    CompPairSnd<model::HashType>());
  
  while (!q.empty())
  {
    model::HashType mNode = q.front();
    q.pop();
    
    auto range = std::equal_range(inheritance.begin(), inheritance.end(), mNode,
      CompPairSnd<model::HashType>());
    
    for (auto p = range.first; p != range.second; ++p)
    {
      const auto& sourceIt = visitedNodes.find(p->first);
      if (sourceIt == visitedNodes.end())
      {
        util::Graph::Node gNode = graph.addNode();
        inhEdges.push_back(graph.addEdge(gNode, visitedNodes[p->second]));
        visitedNodes[p->first] = gNode;
        q.push(p->first);
      }
    }
  }
  
  //--- Collect aggregate nodes ---//
  
  q = qOrig;
  
  typedef odb::query<model::CppMemberType> MTQuery;
  
  transaction([&, this]{
    while (!q.empty())
    {
      model::HashType mNode = q.front();
      q.pop();

      auto memberTypes = db->query<model::CppMemberType>(
        MTQuery::typeHash == mNode &&
        MTQuery::kind == model::CppMemberType::Kind::Field);

      for (const model::CppMemberType& memberType : memberTypes)
      {
        auto typeHash = getInitialTypeOfTypedef(memberType.memberTypeHash);
        const auto& sourceIt = visitedNodes.find(typeHash);
        
        if (sourceIt == visitedNodes.end())
        {
          util::Graph::Node gNode = graph.addNode();
          aggrEdges.push_back(graph.addEdge(gNode, visitedNodes[mNode]));
          visitedNodes[typeHash] = gNode;
        }
      }
    }
  });
  
  //--- Collect reverse aggregation ---//
  
  q = qOrig;
  
  transaction([&, this]{
    while (!q.empty())
    {
      model::HashType mNode = q.front();
      q.pop();
      
      auto memberTypes = db->query<model::CppMemberType>(
        MTQuery::memberTypeHash == mNode &&
        MTQuery::kind == model::CppMemberType::Kind::Field);
      
      for (const model::CppMemberType& memberType : memberTypes)
      {
        const auto& targetIt = visitedNodes.find(memberType.typeHash);
        
        if (targetIt == visitedNodes.end())
        {
          util::Graph::Node gNode = graph.addNode();
          aggrEdges.push_back(graph.addEdge(visitedNodes[mNode], gNode));
          visitedNodes[memberType.typeHash] = gNode;
        }
      }
    }
  });
  
  //--- Decorate nodes ---//
  
  std::vector<util::Graph::Node> nodesToDelete;
  
  transaction([&, this]{
    for (const auto& p : visitedNodes)
    {
      typedef odb::query<model::CppAstNode> NodeQuery;
      auto cppAstNodes = db->query<model::CppAstNode>(
         NodeQuery::mangledNameHash == p.first &&
        (NodeQuery::symbolType == model::CppAstNode::SymbolType::Type ||
         NodeQuery::symbolType == model::CppAstNode::SymbolType::Enum) &&
         NodeQuery::astType == model::CppAstNode::AstType::Definition);
      
      if (cppAstNodes.empty()) {
        nodesToDelete.push_back(p.second);
        continue;
      }
      
      model::CppAstNode cppAstNode = *cppAstNodes.begin();
      
      graph.setAttribute(p.second, "id", std::to_string(cppAstNode.id));
      graph.setAttribute(p.second, "label", cppAstNode.astValue);
      graph.setAttribute(p.second, "shape", "box");
    }
  });
  
  //--- Decorate edges ---//
  
  for (const util::Graph::Edge& edge : inhEdges)
  {
    graph.setAttribute(edge, "color", "red");
    graph.setAttribute(edge, "arrowhead", "empty");
  }
  
  for (const util::Graph::Edge& edge : aggrEdges)
    graph.setAttribute(edge, "arrowhead", "diamond");
  
  //--- Remove unnecessary nodes ---//
  
  for (const util::Graph::Node& node : nodesToDelete)
    graph.delNode(node);
  
  return graph.nodeCount() == 0 ? "" : graph.output(util::Graph::SVG);
}

std::string CppServiceHelper::getClassCollaborationDiagram(
  const core::AstNodeId& astNodeId)
{
  typedef std::pair<util::Graph::Node, util::Graph::Node> GraphNodePair;

  using namespace model;

  util::Graph graph;
  graph.setAttribute("rankdir", "BT");

  std::map<model::CppAstNodeId, util::Graph::Node> visitedNodes;
  std::map<GraphNodePair, util::Graph::Edge> visitedEdges;

  // max depth init
  //int level = 0;
  //static const int MAX_DEPTH = 2;
  std::queue<CppAstNode> wnActualLevel;
  // waiting nodes for processing in the next level
  std::queue<CppAstNode> wnNextLevel;

  std::vector<model::CppAstNode::pktype> processedNodes; // Nodes which are alredy processed
  
  enum Direction {BASE, DERIVED, BOTH};
  
  model::CppAstNode thisAstNode;
  transaction([&, this](){
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    thisAstNode = query.loadDefinition(astNode); });

  //--- Helper functions ---//

  auto createNode = [&graph, &visitedNodes](
    const model::CppAstNode& representedAstNode,
    bool current = false,
    const model::CppAstNodePtr dataAstNode = nullptr)
  {
    std::string label;
    model::CppAstNode::pktype visitedNodeId;
    if (dataAstNode)
    {
      label = dataAstNode->astValue;
      visitedNodeId = dataAstNode->id;
    }
    else
    {
      label = representedAstNode.astValue;
      visitedNodeId = representedAstNode.id;
    }

    util::Graph::Node node = graph.addNode();
    graph.setAttribute(node, "id", std::to_string(representedAstNode.id));
    graph.setAttribute(node, "shape", "box");
    graph.setAttribute(node, "label", label);
    if (current)
    {
      graph.setAttribute(node, "style", "filled");
      graph.setAttribute(node, "fillcolor", "gold");
    }

    visitedNodes[{ visitedNodeId }] = node;

    return node;
  };

  // almost copy paste collectAggregation
  std::function<void(const CppAstNode&)> collectUsedMembers =
    [&, this](const CppAstNode& node) {

    CppType cppType;
    try
    {
      cppType = query.queryEntityByHash<CppType>(node.mangledNameHash);
    }
    catch (std::exception& ex)
    {
      SLog(util::ERROR) << ex.what();
      return;
    }

    auto result = query.queryMembersByType(cppType, CppMemberType::Kind::Field);

    /*if (level == MAX_DEPTH && !result.empty())
    {
      graph.setAttribute(visitedNodes.at({ node.id }), "color", "red");
      return;
    }*/

    for (const CppMemberType& memberType : result)
    {
        try
        {
          CppVariable variable =
            query.queryEntityByHash<CppVariable>(
              memberType.memberAstNode.load()->mangledNameHash);
          
          auto typeHash = getInitialTypeOfTypedef(memberType.memberTypeHash);
          auto type = query.queryEntityByHash<CppType>(typeHash);

          auto typeAstNode = db->load<CppAstNode>(type.astNodeId.get());
          
          // Members declared with type alias should be handled differently as
          // declaration with the original type.
          auto boxAstNode = typeAstNode;
          if (typeHash != memberType.memberTypeHash)
          {
            auto tdef = query.queryEntityByHash<CppTypedef>(
              memberType.memberTypeHash);
            boxAstNode = db->load<CppAstNode>(tdef.astNodeId.get());
          }

          if (typeAstNode->mangledName == "PrimitiveTypesOwner")
            continue;
          
          util::Graph::Node& lhs_node = visitedNodes.at({ node.id });
          util::Graph::Node  rhs_node;
          if (visitedNodes.find({ boxAstNode->id }) == visitedNodes.end())
            rhs_node = createNode(*typeAstNode, false, boxAstNode);
          else
            rhs_node = visitedNodes.at({ boxAstNode->id });

          if (visitedEdges.find(GraphNodePair(lhs_node, rhs_node)) ==
            visitedEdges.end())
          {
            util::Graph::Edge edge = graph.addEdge(lhs_node, rhs_node);
            visitedEdges.emplace(GraphNodePair(lhs_node, rhs_node), edge);
            
            graph.setAttribute(edge, "arrowhead", "normal");
            graph.setAttribute(edge, "style", "dashed");
            graph.setAttribute(edge, "color", "mediumpurple");
            graph.setAttribute(edge, "label", variable.name);
          }
          else
          {

            auto& edge = visitedEdges[GraphNodePair(lhs_node, rhs_node)];
            std::string oldLabel = graph.getAttribute(edge, "label");
            graph.setAttribute(edge, "label", oldLabel + ", " + variable.name);
          }

          
          if(std::find(
            processedNodes.begin(), processedNodes.end(), typeAstNode->id) == processedNodes.end())
          {            
            wnNextLevel.emplace(*boxAstNode);
            processedNodes.push_back(typeAstNode->id);
          }
        }
        catch(const std::exception& ex)
        {
          SLog(util::ERROR) << ex.what();
        }
    }
  };

  std::function<void(const CppAstNode&)> collectBase =
    [&, this](const CppAstNode& node) {
    typedef odb::query<model::CppInheritance> QInheritance;

    CppType cppType;
    try
    {
      cppType = query.queryEntityByHash<CppType>(node.mangledNameHash);
    }
    catch (std::exception& ex)
    {
      SLog(util::ERROR) << ex.what();
      return;
    }
    
    auto result = db->query<CppInheritance>(
      QInheritance::derived == cppType.mangledNameHash);

    /*if (level == MAX_DEPTH && !result.empty())
    {
      graph.setAttribute(visitedNodes.at({ node.id}), "color", "red");
      return;
    }*/

    for (const CppInheritance& inheritance : result)
    {
      try
      {
        auto base = query.queryEntityByHash<CppType>(inheritance.base);
        auto classAstNode = db->load<CppAstNode>(base.astNodeId.get());

        util::Graph::Node& lhs_node = visitedNodes.at({ node.id });
        util::Graph::Node  rhs_node;
        if (visitedNodes.find({ classAstNode->id }) == visitedNodes.end())
          rhs_node = createNode(*classAstNode, false);
        else
          rhs_node = visitedNodes.at({ classAstNode->id });

          util::Graph::Edge edge = graph.addEdge(lhs_node, rhs_node);
          graph.setAttribute(edge, "arrowhead", "normal");
          
          switch (inheritance.visibility)
          {
            case Visibility::Public:
              graph.setAttribute(edge, "color", "darkblue");
            break;

            case Visibility::Protected:
              graph.setAttribute(edge, "color", "darkgreen");
            break;

            case Visibility::Private:
              graph.setAttribute(edge, "color", "darkred");
            break;
          }
          
          
          if(std::find(
            processedNodes.begin(), processedNodes.end(), classAstNode->id) == processedNodes.end())
          {             
            wnNextLevel.emplace(*classAstNode);
            processedNodes.push_back(classAstNode->id);
          }
      }
      catch(const std::exception& ex)
      {
        SLog(util::ERROR) << ex.what();
      }
    }
  };
  
  wnActualLevel.emplace(thisAstNode);
  transaction([&, this]()
  {
    createNode(thisAstNode, true);
    do
    {
      while (!wnActualLevel.empty())
      {
        const auto actNode = wnActualLevel.front();

        collectBase(actNode);
        collectUsedMembers(actNode);

        wnActualLevel.pop();
      }

      wnNextLevel.swap(wnActualLevel);
    } while(/*++level <= MAX_DEPTH &&*/ !wnActualLevel.empty());
  });

  //--- Starting DFS ---//


  return graph.nodeCount() == 0 ? "" : graph.output(util::Graph::SVG);
}

std::string CppServiceHelper::getFunctionCallDiagram(
  const core::AstNodeId& astNodeId)
{
  using namespace model;
  using namespace util;

  Graph graph;
  graph.setAttribute("rankdir", "LR");

  std::map<std::string, Graph::Subgraph> files;
  std::map<std::pair<CppAstNode, std::string>, Graph::Node> graphNode;

  //--- Helper functions ---//

  auto addFileSubgraphIfNeeded =
    [&files, &graph, this](const model::CppAstNode& astNode) {
      std::string filename;

      if (astNode.location.file)
        filename = astNode.location.file.load()->path;
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

  auto getNodeLabel = [this](const CppAstNode& astNode) {
    return query.queryEntityByHash<CppFunction>(
      query.loadDefinitionOrDeclaration(astNode)[0].mangledNameHash
      ).qualifiedName;
  };

  auto addNode = [&, this](Graph& g,
                           const CppAstNode& astNode,
                           const std::string& label) {

    auto nodeAndLabel = make_pair(astNode, label);
    if (graphNode.find(nodeAndLabel) != graphNode.end())
      return graphNode[nodeAndLabel];

    util::Graph::Node node = g.addNode(
      files[addFileSubgraphIfNeeded(astNode)]);

    g.setAttribute(node, "label", label);
    g.setAttribute(node, "id", std::to_string(astNode.id));

    graphNode[nodeAndLabel] = node;

    return node;
  };

  auto addCallColor = [&] (const util::Graph::Node& node, bool called) {
    std::string outColor = "lightblue";
    std::string inColor = "coral";

    std::string fillcolor = called ? outColor : inColor;

    graph.setAttribute(node, "style", "filled");

    auto clr = graph.getAttribute(node, "fillcolor");

    if (!clr.empty() && clr != outColor && clr != inColor)
    {
      return;
    }

    if (clr.empty())
    {
      graph.setAttribute(node, "fillcolor", fillcolor);
    }
    else
    {
      graph.setAttribute(node, "fillcolor", "coral:lightblue");
    }
  };

  CppAstNode centerNode;

  transaction(
    [&, this]()
    {
      auto astNode = query.loadAstNode(astNodeId.astNodeId);
      centerNode = query.loadDefinitionOrDeclaration(astNode)[0];

      auto centerGraphNode = addNode(graph, centerNode, getNodeLabel(centerNode));
      graph.setAttribute(centerGraphNode, "style", "filled");
      graph.setAttribute(centerGraphNode, "fillcolor", "gold");

      auto staticCalls =
        [&, this](odb::result<CppAstNode> calls, bool out)
      {
        std::set<CppAstNode> visited;

        for (auto staticCall : calls)
        {
          try
          {
            auto defNode = out ?
              query.loadDefinitionOrDeclaration(staticCall)[0] :
              query.loadOuterFunction(staticCall);

            if (visited.find(defNode) != visited.end())
            continue;

            visited.insert(defNode);

            auto callGraphNode = addNode(graph, defNode, getNodeLabel(defNode));
            addCallColor(callGraphNode, out);

            Graph::Edge edge;
            if (out)
            {
              edge = graph.addEdge(centerGraphNode, callGraphNode);
            }
            else
            {
              edge = graph.addEdge(callGraphNode, centerGraphNode);
            }

            graph.setAttribute(edge, "color", out ? "blue" : "red");

          }
          catch (const CppOdbQuery::NoOuterFunction& ex)
          {
            auto node = addNode(graph, staticCall, "Global Scope");
            addCallColor(node, false);

            graph.setAttribute(node, "shape", "star");

            auto edge = graph.addEdge(node, centerGraphNode);
            graph.setAttribute(edge, "color", "red");
          }
          catch (const std::runtime_error& ex)
          {
            SLog(ERROR)
              << "Exception: " << ex.what();
          }
        }
      };

      staticCalls(db->query(CppOdbQuery::queryStaticCallsInAstNode(centerNode)), true);
      staticCalls(db->query(CppOdbQuery::queryStaticCallers(centerNode)), false);

      // dynamic calls
      {
        auto nodes =
          db->query(CppOdbQuery::queryDynamicCallsInAstNode(centerNode));

        std::set<unsigned long long> visited;

        for (const auto& node : nodes)
        {
          try
          {
            if (visited.find(node.mangledNameHash) != visited.end())
              continue;

            visited.insert(node.mangledNameHash);

            auto definition = query.loadDefinitionOrDeclaration(node)[0];

            std::string nodeLabel = "Virtual " + getNodeLabel(definition);

            util::Graph::Node callGraphNode = addNode(graph, definition, nodeLabel);
            graph.setAttribute(callGraphNode, "shape", "diamond");
            graph.setAttribute(callGraphNode, "style", "filled");
            graph.setAttribute(callGraphNode, "fillcolor", "cyan");

            auto edge = graph.addEdge(centerGraphNode, callGraphNode);
            graph.setAttribute(edge, "color", "blue");

            auto overriders =
            query.getTransitiveClosureOfRel(CppRelation::Kind::Override,
                                            node.mangledNameHash);

            for (auto overrider : overriders)
            {
              if (visited.find(overrider) != visited.end())
                continue;

              visited.insert(overrider);

              auto overriderFunc = query.queryEntityByHash<CppFunction>(overrider);

              auto overriderNode = *db->load<CppAstNode>(
                overriderFunc.astNodeId.get());

              util::Graph::Node dynGraphNode =
                addNode(graph, overriderNode, overriderFunc.qualifiedName);
              addCallColor(dynGraphNode, true);

              auto edge = graph.addEdge(callGraphNode, dynGraphNode);
              graph.setAttribute(edge, "style", "dashed");
              graph.setAttribute(edge, "color", "blue");
            }

          } catch (std::runtime_error& err)
          {
            SLog(ERROR) << "Exception: " << err.what();
          }
        }

      }

      // dynamic callers
      {
        auto overriddens = query.reverseTransitiveClosureOfRel(
          CppRelation::Kind::Override, centerNode.mangledNameHash);

        overriddens.insert(centerNode.mangledNameHash);

        std::set<unsigned long long> visited;

        for (auto overridden : overriddens)
        {
          try
          {
            if (visited.find(overridden) != visited.end())
            continue;

            visited.insert(overridden);

            auto overriddenFunc = query.queryEntityByHash<CppFunction>(
              overridden);
            auto overriddenNode = *db->load<CppAstNode>(
              overriddenFunc.astNodeId.get());

            auto dynamicCallerOfOverridden =
            db->query(CppOdbQuery::queryDynamicCallers(overriddenNode));

            std::set<CppAstNode> visited;

            for (auto dynamicCaller : dynamicCallerOfOverridden)
            {
              auto caller = query.loadOuterFunction(dynamicCaller);

              if (visited.find(caller) != visited.end())
              continue;

              visited.insert(caller);

              auto callerGraphNode = addNode(graph, caller, getNodeLabel(caller));
              addCallColor(callerGraphNode, false);

              auto edge = graph.addEdge(callerGraphNode, centerGraphNode);
              graph.setAttribute(edge, "style", "dashed");
              graph.setAttribute(edge, "color", "red");
            }
          }
          catch (const std::runtime_error& ex)
          {
            SLog(ERROR)
            << "Exception: " << ex.what();
          }
        }
      }

    });

  return graph.nodeCount() == 0 ? "" : graph.output(util::Graph::SVG);
}

std::vector<AstNodeInfo> CppServiceHelper::getCallerFunctions(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return transaction([&, this]()
  {
    model::FileId fid = stoull(fileId.fid);
    
    std::vector<AstNodeInfo> ret;
    
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    
    auto sHandler = SymbolHandler::getHandler(db, getNodeKind(astNodeId));
    auto callers = sHandler->getCallers(astNode.mangledNameHash, fid);
    
    for (auto call : callers)
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });  
}

std::vector<AstNodeInfo> CppServiceHelper::getFunctionAssigns(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    for (auto call : db->query<model::CppAstNode>(
      CppOdbQuery::queryReads(astNode) &&
      CppOdbQuery::astFileId(stoull(fileId.fid))))
    {
      ret.push_back(createAstNodeInfo(call));
    }

    return ret;
  });
}

core::RangedHitCountResult
CppServiceHelper::makeRangedHitCountResult(
  odb::result<model::AstCountGroupByFiles> resultSet,
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

core::RangedHitCountResult CppServiceHelper::getReferencesPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo)
{
  typedef odb::query<model::AstCountGroupByFiles> ACQuery;

  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto resultSet = db->query<model::AstCountGroupByFiles>(
      ACQuery::CppAstNode::mangledNameHash == astNode.mangledNameHash
    );

    return makeRangedHitCountResult(resultSet, pageSize, pageNo);
  });
}

core::RangedHitCountResult CppServiceHelper::getCallerFunctionsPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo)
{
  typedef odb::query<model::AstCountGroupByFiles> ACQuery;

  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto resultSet = db->query<model::AstCountGroupByFiles>(
      ACQuery::CppAstNode::mangledNameHash == astNode.mangledNameHash &&
      ( ACQuery::CppAstNode::astType == model::CppAstNode::AstType::Usage || 
        ACQuery::CppAstNode::astType == model::CppAstNode::AstType::VirtualCall)
    );

    return makeRangedHitCountResult(resultSet, pageSize, pageNo);
  });
}

core::RangedHitCountResult CppServiceHelper::getFunctionAssignsPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo)
{
  typedef odb::query<model::AstCountGroupByFiles> ACQuery;

  return transaction([&, this]()
  {
    auto astNode = query.loadAstNode(astNodeId.astNodeId);
    auto resultSet = db->query<model::AstCountGroupByFiles>(
      ACQuery::CppAstNode::mangledNameHash == astNode.mangledNameHash &&
      ACQuery::CppAstNode::astType == model::CppAstNode::AstType::Read
    );

    return makeRangedHitCountResult(resultSet, pageSize, pageNo);
  });
}

std::vector<AstNodeInfo> CppServiceHelper::getReferences(
  const core::AstNodeId& astNodeId)
{
  return getReferences(astNodeId, nullptr);
}

std::vector<AstNodeInfo> CppServiceHelper::getReferences(
  const core::AstNodeId& astNodeId,
  const core::FileId& fileId)
{
  return getReferences(astNodeId, std::make_unique<core::FileId>(fileId));
}

std::vector<AstNodeInfo> CppServiceHelper::getReferences(
  const core::AstNodeId& astNodeId,
  const std::unique_ptr<core::FileId>& fileId)
{
  return transaction([&, this]()
  {
    std::vector<AstNodeInfo> ret;
    auto astNode = query.loadAstNode(astNodeId.astNodeId);

    auto addRefs = [&](
      const model::HashType& mangledNameHash,
      const std::function<bool(const model::CppAstNode&)> filter = 
        [](const model::CppAstNode&){ return true; })
    {
      auto q = odb::query<model::CppAstNode>(
        CppOdbQuery::astMangledNameHash(mangledNameHash));
      if (fileId != nullptr)
        q = q && CppOdbQuery::astFileId(stoull(fileId->fid));

      for (auto ref : db->query(q))
      {
        if (!filter(ref))
          continue;

        const auto& info = createAstNodeInfo(ref);
        ret.push_back(info);
      }
    };

    // normal references
    addRefs(astNode.mangledNameHash);

    // virtual calls
    auto relatedMangledNameHashes = query.reverseTransitiveClosureOfRel(
                                model::CppRelation::Kind::Override,
                                astNode.mangledNameHash);

    for (auto& mangledNameHash : relatedMangledNameHashes)
    {
      addRefs(mangledNameHash, [](const model::CppAstNode& astNode)
        {
          return astNode.astType == model::CppAstNode::AstType::VirtualCall;
        });
    }
    
    auto overriders = query.getTransitiveClosureOfRel(
      model::CppRelation::Kind::Override, astNode.mangledNameHash);

    for (const auto& overrider : overriders)
    {
      auto overFunc = query.queryEntityByHash<model::CppFunction>(overrider);

      for (auto ref : db->query<model::CppAstNode>(
                          CppOdbQuery::astMangledNameHash(overFunc.mangledNameHash)))
      {
        const auto& info = createAstNodeInfo(ref);
        ret.push_back(info);
      }
    }

    return ret;
  });
}

// ----------------- SLICING FUNCTIONS ---------------------------

void CppServiceHelper::getBackwardSlicePos( 
  std::vector<core::Range>& _return,
  const core::FilePosition& filePos)
{
  SlicerHelper sh(db);
  sh.getBackwardSlicePos(_return, filePos);
}
  
void CppServiceHelper::getForwardSlicePos(
  std::vector<core::Range>& _return,
  const core::FilePosition& filePos)
{
  SlicerHelper sh(db);
  sh.getForwardSlicePos(_return, filePos);
}

// ----------------- END OF SLICING FUNCTIONS ---------------------

// ----------------- START SYNTAX HIGHLIGHT ---------------------
std::vector<core::SyntaxHighlight> CppServiceHelper::getSyntaxHighlight(
        const core::FileId& fileId) 
{
  typedef odb::query<model::CppAstNodeId> AQ;
  
  return transaction([&, this]() 
  {
    std::vector<core::SyntaxHighlight> ret;
    std::vector < std::string > fileLinesContent; // Actual text file lines    

    // Load file content  
    model::File f;
    db->find<model::File>(stoull(fileId.fid), f);
    f.content.load();

    if (!f.content)
      return ret;
    
    // Break it into lines
    std::istringstream s(f.content->content);
    std::string line;
    while (std::getline(s, line)) 
    {
      fileLinesContent.push_back(line);
    }
    
    // get size of astnode astValue in str from startPos
    auto getRealAstNodeLength = [](const std::string& str, const int& startPos,
      const int& startLine, const int& startCol,
      const int& endLine, const int& endCol)
    {      
      int size = 0;
      int sLine = startLine;
      int sCol = startCol;
      while(sLine < endLine || (sLine == endLine && sCol <= endCol))
      {
        if(str[size + startPos] == '\n')
        {
          sCol = 1;
          ++sLine;
        }
        ++sCol;
        ++size;
      }
      return size;
    };
    
    // get function parameter syntax
    auto getParamsSyntax = [fileLinesContent, fileId, getRealAstNodeLength, this](std::string str,
      const int& startLine, const int& startCol, 
      const int& endLine, const int& endCol, const model::CppAstNode& functionAstNode)
    {
      // get function definition
      auto defNode = db->query<model::CppAstNode>(
          CppOdbQuery::astMangledNameHash(functionAstNode.mangledNameHash) &&
          CppOdbQuery::astAstType(model::CppAstNode::AstType::Definition));
          
      int paramCount = 0; // num of parameter
      int openBracketCount = 0; // num of open round brackets
      std::vector<core::SyntaxHighlight> paramsSyntax; // function parameters syntax highlight
            
      int startParamLine = startLine;
      int startParamCol = startCol;
      int strPos = startCol;
      
      while(startParamLine < endLine || (
            startParamLine == endLine && startParamCol <= endCol))
      {
        while(str[strPos] == ',' || str[strPos] == ' ' || // skip white spaces and commas
              str[strPos] == '\t' || str[strPos] == '\n' )
        {
          if(str[strPos] == '\n')
          {
            startParamCol = 1;
            ++startParamLine;
          }else
          {
            ++startParamCol;
          }          
          ++strPos;
        }
        
        if(str[strPos] == '(')
          ++openBracketCount;
        
        if(str[strPos] == ')' && openBracketCount > 0)
          --openBracketCount;
        
        if((str[strPos] == ')' && openBracketCount == 0 ) || (
            startParamLine == endLine && startParamCol >= endCol)) //end of function call
        {
          break;
        }  
        
        model::Position position;
        position.line = startParamLine;
        position.column = startParamCol;

        // get biggest range of astNode in position
        auto result = db->query<model::CppAstNode>(
          (CppOdbQuery::astFileId(stoull(fileId.fid)) && 
           CppOdbQuery::astStartWithRange(position)) + (
           "ORDER BY" + AQ::location.range.end.column + "DESC"));
        
        core::SyntaxHighlight syntax; 
        syntax.className = "cm-Function-Param";
        if(!defNode.empty())  
        {
          auto defParameters = query.queryEntityByHash<model::CppFunction>(defNode.begin()->mangledNameHash).parameters;
          try{
            defParameters.at(paramCount).load();   
            if(defParameters[paramCount]->qualifiedType.find("&") != std::string::npos)
            {
              syntax.className += " cm-Function-Ref-Param";
            }
          }catch(std::exception ex){} // No parameter
        }      
        
        if (result.empty()) // not astNode
        {                
          int nonAstNodeStartCol = startParamCol;
          
          while((unsigned)strPos < str.length() && (
                  str[strPos] != ',' && (
                    str[strPos] != ')' || 
                      (str[strPos] == ')' && openBracketCount > 0))))
          {
            if(str[strPos] == '(')
              ++openBracketCount;
            if(str[strPos] == ')')
              --openBracketCount;
            ++startParamCol;
            ++strPos;
          }
          if(startParamLine < endLine || (
             startParamLine == endLine && startParamCol <= endCol))
          {            
            syntax.range.startpos.line = startParamLine;
            syntax.range.startpos.column = nonAstNodeStartCol;
            syntax.range.endpos.line = startParamLine;
            syntax.range.endpos.column = startParamCol;

            
          }
          ++startParamCol;    
          ++strPos;
          ++paramCount;
        }else // astNode
        {
          auto location = result.begin()->location;
          syntax.range.startpos.line = startParamLine;
          syntax.range.startpos.column = startParamCol;
          syntax.range.endpos.line = location.range.end.line;
          syntax.range.endpos.column = location.range.end.column;
          
          strPos += getRealAstNodeLength(str,strPos, 
            location.range.end.line, location.range.start.column,
            location.range.end.line, location.range.end.column
            );          
          
          startParamCol = location.range.end.column + 1;
          startParamLine = location.range.end.line;
          ++paramCount;
        }
        paramsSyntax.push_back(syntax);   
      }
      return paramsSyntax;
    };
    
    const std::vector<std::string> filters;
    
    for (const auto& ref : db->query<model::CppAstNode>(
      CppOdbQuery::astFileId(stoull(fileId.fid)))
    ) 
    {
      std::string reg = ""; // Regular expression to find element position
      int addToPos = 0; // The number of character in regular expression before real content
      int astValueLength = ref.astValue.length(); // Real content length
      std::string extraClassName = "";
      if (ref.symbolType == model::CppAstNode::SymbolType::Macro) // Macro  
      {   
        reg = ref.astValue;
        extraClassName = "cm-Macro";
      } else if (ref.symbolType == model::CppAstNode::SymbolType::Enum ||  
        ref.symbolType == model::CppAstNode::SymbolType::EnumConstant) 
      {
        reg = ref.astValue;
        extraClassName = "cm-Enum";
      } else if (ref.symbolType == model::CppAstNode::SymbolType::Function)  // Function: Declaration, Definition, Usage, VirtualCall
      { 
        if (ref.astType == model::CppAstNode::AstType::Usage ||
          ref.astType == model::CppAstNode::AstType::VirtualCall) 
        {
          reg = std::string(ref.astValue).erase(0, 5); // remove "call " prefix from content
          astValueLength -= 5;          
        } else 
        {
          reg = ref.astValue;
        }
        reg += "(\n \t)*\\(";
        extraClassName = "cm-Function";
      } else if (ref.symbolType == model::CppAstNode::SymbolType::Variable) // Variable : Declaration, Definition, Read, Write
      { 
        reg = "[^a-zA-Z0-9_]";
        if (ref.astType == model::CppAstNode::AstType::Read ||
          ref.astType == model::CppAstNode::AstType::Write) 
        {
          reg += std::string(ref.astValue).erase(0, 4); // remove "ref " prefix from content
          astValueLength -= 4;
        } else 
        {
          reg += ref.astValue;
        }
        reg += "[^a-zA-Z0-9_]";
          addToPos = 1;
          extraClassName = "cm-Variable";
      } else if (ref.symbolType == model::CppAstNode::SymbolType::Type ||  // Type
        ref.symbolType == model::CppAstNode::SymbolType::Typedef) 
      {
        reg = ref.astValue + "[^a-zA-Z0-9_\\(]";
        extraClassName = "cm-Type";
      }
      if (reg != "") 
      {
        // TODO: The loop variable is stopped at the end of file. For some
        // reason the database sometimes contains nodes of which the start
        // position is greater than the end position. This should be checked in
        // the parser. E.g. Xerces code, DOMNode.hpp getOwnerDocument function.
        // This node is contained several times in the database with same start
        // line position and different end positions.
        for (unsigned int i = ref.location.range.start.line - 1; i < ref.location.range.end.line && i < fileLinesContent.size(); ++i)
        {
          std::regex words_regex(reg);
          auto words_begin =
          std::sregex_iterator(fileLinesContent[i].begin(), fileLinesContent[i].end(), words_regex);
          auto words_end = std::sregex_iterator();

          for (std::sregex_iterator ri = words_begin; ri != words_end; ++ri) 
          {
            std::smatch match = *ri;
            std::string match_str = match.str();

            core::SyntaxHighlight syntax;

            syntax.range.startpos.line = i + 1;
            syntax.range.startpos.column = ri->position() + 1 + addToPos;
            syntax.range.endpos.line = i + 1;
            syntax.range.endpos.column = syntax.range.startpos.column + astValueLength;

            syntax.className = "cm-" + model::CppAstNode::symbolTypeToString(ref.symbolType) + 
              "-" + model::CppAstNode::astTypeToString(ref.astType) + " " + extraClassName;
                     
            ret.push_back(std::move(syntax));
            /*
             Commented out because it was too slow
            */   
//            if(ref.symbolType == model::CppAstNode::SymbolType::Function &&
//               ref.astType == model::CppAstNode::AstType::Usage)
//            {                
//                std::vector<core::SyntaxHighlight> paramsSyntax;
//                
//                std::string functionStr = std::accumulate( 
//                  fileLinesContent.begin() + syntax.range.endpos.line - 1, 
//                  fileLinesContent.begin() + ref.location.range.end.line, std::string("") );
//                
//                paramsSyntax = getParamsSyntax(" " + functionStr, // function string
//                  syntax.range.endpos.line, syntax.range.endpos.column + 1, // end of function name
//                  ref.location.range.end.line, ref.location.range.end.column, // end of function
//                  ref //astNode
//                );
//                for (const auto& paramSyntax : paramsSyntax)
//                {
//                    ret.push_back(std::move(paramSyntax));
//                }
//            }
          }
        }
      }
    }

    return ret;
  });
}


// ----------------- END OF SYNTAX HIGHLIGHT ---------------------

model::HashType CppServiceHelper::getInitialTypeOfTypedef(const model::HashType& td)
{
  using namespace model;
  typedef typename odb::query<CppRelation> Query;

  HashType ret = td;
  
  auto q = odb::query<CppRelation>(
    Query::rhs == Query::_ref(ret) &&
    Query::kind == CppRelation::Kind::Alias);

  auto result = db->query(q);
  while(!result.empty())
  {
    // It should contain only one element.
    auto type = result.begin();

    ret = type->lhs;
    result = db->query(q);
  }

  return ret;
}

std::vector<InfoNode> CppServiceHelper::getCatalogue()
{
  return transaction([&, this]()
  {
    using namespace model;

    //
    // helper definitions

    std::vector<InfoNode> ret;

    auto makeAnonymousNamespace = [&]()
    {
      auto inode = makeInfoNode({}, "(anonymous)", "");
      inode.query.queryId =  1;
      inode.query.filters = { "%(anonymous)" };

      return inode;
    };

    auto makeGlobalNamespace = [&]()
    {
      auto inode = makeInfoNode({}, "(global types)", "");
      inode.query.queryId =  1;
      inode.query.filters = { "globalType" };

      return inode;
    };

    //
    // logic

    ret.push_back(makeAnonymousNamespace());
    ret.push_back(makeGlobalNamespace());

    auto result = collectCatalogueByHitCounter(
      odb::query<CppAstNode>(
        CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Namespace) &&
        !CppOdbQuery::AstQuery::mangledName.like("%::%")));
    
    ret.insert(ret.end(), result.begin(), result.end());

    return ret;
  });
}

std::vector<InfoNode> CppServiceHelper::getSubCatalogue(const InfoQuery& query)
{
  return transaction([&, this]()
  {
    using namespace model;

    std::vector<InfoNode> ret;
    auto q = odb::query<CppAstNode>(
      (
        CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Type) &&
        CppOdbQuery::astAstType(CppAstNode::AstType::Definition)
      ) || 
      CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Namespace)
    );

    std::string mnPrefix = "%";
    auto mangledNamePrefix = query.filters.front();

    if (mangledNamePrefix == "globalType")
    {
      q = odb::query<CppAstNode>(
        CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Type) &&
        CppOdbQuery::astAstType(CppAstNode::AstType::Definition));
    }
    else if (mangledNamePrefix == "")
      q = CppOdbQuery::astSymbolType(CppAstNode::SymbolType::Namespace);
    else
      mnPrefix = query.filters.front() + "::_%";

    q = q && (
      CppOdbQuery::AstQuery::mangledName.like(mnPrefix) &&
      !CppOdbQuery::AstQuery::mangledName.like(mnPrefix + "::%"));      

    auto result = collectCatalogueByHitCounter(q, query);
    
    ret.insert(ret.end(), result.begin(), result.end());

    return ret;
  });
}

std::vector<InfoNode> CppServiceHelper::collectCatalogueByHitCounter(
  odb::query<model::CppAstNode> query,
  const InfoQuery& iquery)
{
  using namespace model;

  std::vector<InfoNode> ret;

  //
  // helper definitions

  static const int MAX_HIT = 15;
  std::set<HashType> visitedNamespaces;

  auto isAnonymousNm = [](const CppAstNode& node)
  {
    return node.symbolType == CppAstNode::SymbolType::Namespace
      && node.astValue == "<anonymous>";
  };

  auto handleNamespace = [&](const CppAstNode& node)
  {
    if (node.symbolType == CppAstNode::SymbolType::Namespace)
    {
      auto nextMangledNameToken = (iquery.filters.size() > 0) ? iquery.filters[0] : "";
      if (isAnonymousNm(node) && nextMangledNameToken != "%(anonymous)")
        return true;
      else if (visitedNamespaces.count(node.mangledNameHash) > 0)
        return true;
      else
        visitedNamespaces.insert(node.mangledNameHash);
    }

    return false;
  };

  auto makeINode = [&](const CppAstNode& node) //TODO: rename to makeInfoNode
  {
    auto inode = makeInfoNode({}, node.astValue, "",
      createAstNodeInfo(node));
    inode.astValue.documentation
      = getDocCommentInTransaction(node.mangledNameHash);
    inode.query.queryId =  1;
    
    if (isAnonymousNm(node))
    {
      inode.label = "(anonymous)";
      inode.query.filters = { "%(anonymous)" };
    }
    else
      inode.query.filters = { node.mangledName, "0" };

    return inode;
  };

  auto createMoreBtn = [&](
    const int offset)
  {
    static unsigned int moreBtnCounter = 0;

    InfoNode inode;
    inode.astValue.astNodeId.astNodeId = "more " + std::to_string(moreBtnCounter++);
    inode.astValue.astNodeType = "MoreBtn";

    auto nextMangledNameToken = (iquery.filters.size() > 0) ? iquery.filters[0] : "";

    inode.query.queryId =  /*(nextMangledNameToken == "") ? 0 :*/ 1;
    inode.query.filters = { nextMangledNameToken, std::to_string(offset) };

    return inode;
  };

  //
  // logic

  auto offset = (iquery.filters.size() > 1)
      ? std::stoi(iquery.filters[1])
      : 0;

  bool noMoreItems = false;
  int hitCounter = 0;
  int visited = 0;

  for (auto& node : db->query(odb::query<CppAstNode>(query +
    " ORDER BY " + CppOdbQuery::AstQuery::mangledName)))
  {
    if (visited++ < offset)
      continue;

    if (hitCounter == MAX_HIT)
      break;

    if (handleNamespace(node))
      continue;

    ret.emplace_back(makeINode(node));
    ++hitCounter;
  }

  if (hitCounter < MAX_HIT)
    noMoreItems = true;

  if (!noMoreItems)
  {
    auto moreBtn = createMoreBtn(visited);
    ret.emplace_back(moreBtn);
  }

  return ret;
}

} // language
} // service
} // cc

