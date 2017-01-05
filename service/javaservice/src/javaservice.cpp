// $Id$
// Created by Aron Barath, 2013

#include "javaservice.h"
#include <util/streamlog.h>
#include "util/standarderrorlogstrategy.h"
#include "language-api/LanguageService.h"

#include "model/java/javaastnode.h"

// for now we only support one workspace 
#define WORKSPACE_ID 1

namespace cc
{
namespace service
{
namespace language 
{

  JavaServiceHandler::JavaServiceHandler(const JavaServiceHelper& helper)
  : helper(helper)
  {
  }

  void JavaServiceHandler::getAstNodeInfoByPosition(AstNodeInfo& _return,
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters)
  {
    SLog(util::DEBUG)<<"getting astnode info by position for file:"<<fpos.file.fid<<"line:"<<fpos.pos.line<<"col:"<<fpos.pos.column;
    _return = helper.getAstNodeInfoByPosition(fpos, filters);
  }
  
  void JavaServiceHandler::getInfoBox(InfoBox& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getInfoBox(astNodeId);
  }
  
  void JavaServiceHandler::getInfoBoxByPosition(InfoBox& _return,
      const core::FilePosition& fpos,
      const std::vector<std::string> & filters)
  {
    _return = helper.getInfoBoxByPosition(fpos, filters);
  }

  void JavaServiceHandler::getAstNodeInfo(AstNodeInfo& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getAstNodeInfo(astNodeId);
  }
  
 /* void JavaServiceHandler::getDiagramTypes(std::vector< core::DiagramType> & _return,
    const core::AstNodeId& astNodeId)
  {
	  printf("WARNING \n dead code is called,"
	 			  "(getDiagramTypes) malfunctions expected \n");

    core::DiagramType type;
    
    type.diagramId = JavaServiceHelper::FUNCTION_CALL;
    type.diagramName = "Function Call Diagram";
    
    _return.push_back(type);
    
    printf("getDiagramTypes\n");
  }*/

  void JavaServiceHandler::getDiagram(std::string& _return,
    const core::AstNodeId& astNodeId, const core::DiagramId::type diagramId)
  {
    _return = helper.getDiagram(astNodeId, diagramId);
  }

  void JavaServiceHandler::getLegend(std::string& _return,
    const core::DiagramId::type diagramId)
  {
    _return = helper.getLegend(diagramId);
  }

  void JavaServiceHandler::getFunctionCallPathDiagram(std::string& _return,
    const core::AstNodeId& astNodeIdFrom,
    const core::AstNodeId& astNodeIdTo)
  {
    _return = helper.getDiagram(astNodeIdFrom, astNodeIdTo);
  }
  
  /*void JavaServiceHandler::getReferenceTypes(
		  std::vector< core::ReferenceType> & _return,
		  const core::AstNodeId& astNodeId)
  {
	  printf("WARNING \n dead code is called,"
			  "(getReferenceTypes) malfunctions expected \n");
	  for (const auto& refPair : referenceType)
    {
      _return.push_back(refPair.second);
    }
  }*/
  
  void JavaServiceHandler::getReferences(std::vector<AstNodeInfo> & _return,
      const core::AstNodeId& astNodeId,
      const RefTypes::type referenceId)
  {
    switch(referenceId)
    {
    case RefTypes::GetDef:
      _return.push_back(helper.getDefinition(astNodeId));
      break;
    case RefTypes::GetFuncCalls:
      _return = helper.getFunctionCalls(astNodeId);
      break;
    default:
      break;
    }
  }

  void JavaServiceHandler::getReferencesInFile(std::vector<AstNodeInfo> & _return,
    const core::AstNodeId& astNodeId, const RefTypes::type referenceId,
    const core::FileId& fileId)
  {
    switch(referenceId)
    {
    case RefTypes::GetUsage:
      _return = helper.getReferences(astNodeId, fileId);
      break;
    case RefTypes::GetCallerFuncs:
      _return = helper.getCallerFunctions(astNodeId, fileId);
      break;
    default:
      break;
    }
  }

  void JavaServiceHandler::getPage(
    core::RangedHitCountResult& _return,
    const core::AstNodeId& astNodeId,
    const RefTypes::type referenceId,
    const int32_t pageSize,
    const int32_t pageNo)
  {
    switch(referenceId)
    {
    case RefTypes::GetUsage:
      _return = helper.getReferencesPage(astNodeId, pageSize, pageNo);
      break;
    case RefTypes::GetCallerFuncs:
      _return = helper.getCallerFunctionsPage(astNodeId, pageSize, pageNo);
      break;
    default:
      break;
    }
  }

  void JavaServiceHandler::getTypeDefinitions(
    std::vector<core::AstNodeId> & _return,
    const std::string& path)
  {
    // TODO: implement
    // _return = helper.getTypeDefinitions(path);
  }

  void JavaServiceHandler::getSourceCode(std::string& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getSourceCode(astNodeId);
  }

  void JavaServiceHandler::getDocComment(std::string& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getDocComment(astNodeId);
  }

  void JavaServiceHandler::getMenuTypes(std::vector< core::MenuType> & _return,
                                       const core::AstNodeId& astNodeId) {
    SLog() << "get menu types called";

    const auto nodeKind = helper.getNodeKind(astNodeId);
    
    if (nodeKind == model::SymbolType::File)
      return;

    core::MenuType tmpMenuType;
    tmpMenuType.astNodeId = helper.getDefinition(astNodeId).astNodeId;
    
    //--- Jump to definition ---//
    
    tmpMenuType.category = core::Category::jumpToDef;
    tmpMenuType.name = {"Jump to definition"};
    tmpMenuType.menuItemId = 0;
    tmpMenuType.helpText = "";
    tmpMenuType.shortcut = "ctrl - click";
    _return.push_back(tmpMenuType);
    
    //--- Info Tree ---//
            
    tmpMenuType.category = core::Category::infoTree;
    tmpMenuType.name = {"Info Tree"};
    tmpMenuType.menuItemId = 0;
    tmpMenuType.helpText = "";
    tmpMenuType.shortcut = "";
    _return.push_back(tmpMenuType);
    
    //--- Info Box ---//

    tmpMenuType.category = core::Category::infoBox;
    tmpMenuType.name = {"Info Box"};
    tmpMenuType.menuItemId = 0;
    tmpMenuType.helpText = "";
    tmpMenuType.shortcut = "ctrl - rightclick";
    _return.push_back(tmpMenuType);
    
    //--- References ---//
    
    tmpMenuType.category = core::Category::references;
    tmpMenuType.name = {"References", "Get definition"};
    tmpMenuType.menuItemId = RefTypes::GetDef;
    tmpMenuType.helpText = "";
    tmpMenuType.shortcut = "";
    _return.push_back(tmpMenuType);

    if (nodeKind != model::SymbolType::Function)
    {
      tmpMenuType.category = core::Category::pagingResult;
      tmpMenuType.name = {"References", "Get usage"};
      tmpMenuType.menuItemId = RefTypes::GetUsage;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);
    }

    if (nodeKind == model::SymbolType::Function)
    {
      //--- References ---//

      tmpMenuType.category = core::Category::references;
      tmpMenuType.name = {"References", "Get Called Functions"};
      tmpMenuType.menuItemId = RefTypes::GetFuncCalls;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      tmpMenuType.category = core::Category::pagingResult;
      tmpMenuType.name = {"References", "Get Direct Calls of This"};
      tmpMenuType.menuItemId = RefTypes::GetCallerFuncs;
      tmpMenuType.helpText = "This option doesn't work for virtual functions.";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      //--- Diagrams ---//

      tmpMenuType.category = core::Category::diagram;
      tmpMenuType.name = {"Diagrams", "Function Call Diagram"};
      tmpMenuType.menuItemId = core::DiagramId::FUNCTION_CALL;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

//      tmpMenuType.category = core::Category::diagram;
//      tmpMenuType.name = {"Diagrams", "Call Path Diagram", "To this node"};
//      tmpMenuType.menuItemId = core::DiagramId::FUNCTION_PATH;
//      tmpMenuType.helpText = "";
//      tmpMenuType.shortcut = "";
//      _return.push_back(tmpMenuType);
    }

    if (nodeKind == model::SymbolType::Type)
    {
      //--- Diagrams ---//

      tmpMenuType.category = core::Category::diagram;
      tmpMenuType.name = {"Diagrams", "Class Diagram"};
      tmpMenuType.menuItemId = core::DiagramId::CLASS;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      tmpMenuType.category = core::Category::diagram;
      tmpMenuType.name = {"Diagrams", "Class Hierarchy Diagram"};
      tmpMenuType.menuItemId = core::DiagramId::FULL_CLASS;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);
    }

    //--- CODE BITES ---//

    if (nodeKind == model::SymbolType::Function
      || nodeKind == model::SymbolType::Macro
      || nodeKind == model::SymbolType::Type)
    {
      tmpMenuType.category = core::Category::codebites;
      tmpMenuType.name = {"Diagrams", "CodeBites"};
      tmpMenuType.menuItemId = core::DiagramId::CODE_BITES;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);
    }
  }
  
  void JavaServiceHandler::getFileMenuTypes(std::vector< core::MenuType>& _return,
    const core::FileId& fileId)
  {
    SLog() << "get file menu types called";
    
    core::MenuType tmpMenuType;
    
    //--- Diagrams ---//
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "Build Diagram"};
//    tmpMenuType.menuItemId = core::DiagramId::BUILD;
    _return.push_back(tmpMenuType);
  }
  
  void JavaServiceHandler::getDirMenuTypes(std::vector< core::MenuType>& _return,
    const core::FileId& dirId)
  {
    SLog() << "get directory menu types called";
    // TODO
    printf("JavaServiceHandler::getDirMenuTypes is not implemented\n");
  }

  /*void JavaServiceHandler::getFileDiagramTypes(
    std::vector< core::DiagramType> & _return,
    const core::FileId& fileId)
  {
    // Your implementation goes here
    printf("getFileDiagramTypes\n");
  }*/
  
  void JavaServiceHandler::getFileDiagram(std::string& _return,
    const core::FileId& fileId, const core::DiagramId::type diagramId)
  {
    // TODO
    printf("JavaServiceHandler::getFileDiagram is not implemented\n");
    _return = "";//helper.getFileDiagram(fileId, static_cast<JavaServiceHelper::DiagramType>(diagramId));
  }
  
  /*void JavaServiceHandler::getFileReferenceTypes(
    std::vector< core::ReferenceType> & _return,
    const core::FileId& fileId)
  {
    // Your implementation goes here
    printf("getFileReferenceTypes\n");
  }*/
  
  void JavaServiceHandler::getFileReferences(std::vector<AstNodeInfo> & _return,
    const core::FileId& fileId, const RefTypes::type referenceId)
  {
    // TODO
    printf("JavaServiceHandler::getFileReferences is not implemented\n");
  }

  void JavaServiceHandler::getInfoTree(std::vector<InfoNode> & _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getInfoTree(astNodeId);
  }
  
  void JavaServiceHandler::getSubInfoTree(std::vector<InfoNode> & _return,
      const core::AstNodeId& astNodeId, const InfoQuery& query)
  {
    _return = helper.getSubInfoTree(astNodeId, query);
  }

  void JavaServiceHandler::getCatalogue(std::vector<InfoNode> & _return)
  {
  }

  void JavaServiceHandler::getSubCatalogue(std::vector<InfoNode> & _return,
   const InfoQuery& query)
  {
  }

  void JavaServiceHandler::getInfoTreeForFile(
  std::vector<InfoNode>& _return,
  const core::FileId& fileId)
  {
    _return = helper.getInfoTreeForFile(fileId);
  }

  void JavaServiceHandler::getSubInfoTreeForFile(
      std::vector<InfoNode>& _return,
      const core::FileId& fileId,
      const InfoQuery& query)
  {
    _return = helper.getSubInfoTreeForFile(fileId, query);
  }

  core::AstNodeId JavaServiceHandler::makeAstNodeId( int nodeId_)
  {
    core::AstNodeId id;
    id.astNodeId = std::to_string(nodeId_);
    return id;
  }
  
  void JavaServiceHandler::getBackwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos)
  {
    
  }
  
  void JavaServiceHandler::getForwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos)
  {
    
  }
  
  void JavaServiceHandler::getSyntaxHighlight(
   std::vector<core::SyntaxHighlight>& _return,
   const core::FileId& fileId)
  {
    // TODO
  }
} // language
} // service
} // cc

