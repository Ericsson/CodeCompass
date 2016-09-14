/*
* cppservice.cpp
*
*  Created on: Apr 10, 2013
*      Author: ezoltbo
*/

#include "cppservice.h"
#include <util/streamlog.h>
#include "util/standarderrorlogstrategy.h"
#include "language-api/LanguageService.h"

#include "model/cxx/cppastnode.h"

//for now we only support one workspace 
#define WORKSPACE_ID 1

namespace cc
{ 
namespace service
{  
namespace language
{
  
  CppServiceHandler::CppServiceHandler(const CppServiceHelper& helper)
  : helper(helper)
  {
  }
  
  void CppServiceHandler::getAstNodeInfoByPosition(AstNodeInfo& _return,
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters)
  {
    SLog(util::DEBUG)<<"getting astnode info by position for file:"<<fpos.file.fid<<"line:"<<fpos.pos.line<<"col:"<<fpos.pos.column;
    _return = helper.getAstNodeInfoByPosition(fpos, filters);
  }
  
  void CppServiceHandler::getInfoBox(InfoBox& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getInfoBox(astNodeId);
  }
  
  void CppServiceHandler::getInfoBoxByPosition(InfoBox& _return,
      const core::FilePosition& fpos,
      const std::vector<std::string> & filters)
  {
    _return = helper.getInfoBoxByPosition(fpos, filters);
  }

  void CppServiceHandler::getAstNodeInfo(AstNodeInfo& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getAstNodeInfo(astNodeId);
  }
  
 /* void CppServiceHandler::getDiagramTypes(std::vector< core::DiagramType> & _return,
    const core::AstNodeId& astNodeId)
  {
	  printf("WARNING \n dead code is called,"
	 			  "(getDiagramTypes) malfunctions expected \n");

    core::DiagramType type;
    
    type.diagramId = CppServiceHelper::FUNCTION_CALL;
    type.diagramName = "Function Call Diagram";
    
    _return.push_back(type);
    
    printf("getDiagramTypes\n");
  }*/

  void CppServiceHandler::getDiagram(std::string& _return,
    const core::AstNodeId& astNodeId, const core::DiagramId::type diagramId)
  {
    _return = helper.getDiagram(astNodeId, diagramId);
  }

  void CppServiceHandler::getLegend(std::string& _return,
    const core::DiagramId::type diagramId)
  {
    _return = helper.getLegend(diagramId);
  }

  void CppServiceHandler::getFunctionCallPathDiagram(std::string& _return,
    const core::AstNodeId& astNodeIdFrom,
    const core::AstNodeId& astNodeIdTo)
  {
    _return = helper.getDiagram(astNodeIdFrom, astNodeIdTo);
  }

  /*void CppServiceHandler::getReferenceTypes(
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
  
  void CppServiceHandler::getReferences(std::vector<AstNodeInfo> & _return,
      const core::AstNodeId& astNodeId,
      const RefTypes::type referenceId)
  {
    switch(referenceId)
    {
    case RefTypes::GetDef:
      _return = helper.getDefinition(astNodeId);
      break;
    case RefTypes::GetUsage:
      _return = helper.getReferences(astNodeId);
      break;
    case RefTypes::GetFuncCalls:
      _return = helper.getFunctionCalls(astNodeId);
      break;
    default:
      break;
    }
  }

  void CppServiceHandler::getReferencesInFile(std::vector<AstNodeInfo> & _return,
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
    case RefTypes::GetAssignsToFuncPtrs:
      _return = helper.getFunctionAssigns(astNodeId, fileId);
    default:
      break;
    }
  }
  
  void CppServiceHandler::getPage(
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
    case RefTypes::GetAssignsToFuncPtrs:
      _return = helper.getFunctionAssignsPage(astNodeId, pageSize, pageNo);
      break;
    default:
      break;
    }
  }

  void CppServiceHandler::getSourceCode(std::string& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getSourceCode(astNodeId);
  }
  
  void CppServiceHandler::getDocComment(std::string& _return,
    const core::AstNodeId& astNodeId)
  {
    _return = helper.getDocComment(astNodeId);
  }

  void CppServiceHandler::getMenuTypes(std::vector< core::MenuType> & _return,
                                       const core::AstNodeId& astNodeId) {
    SLog() << "get menu types called";

    const auto nodeKind = helper.getNodeKind(astNodeId);
    
    if (nodeKind == model::CppAstNode::SymbolType::File)
      return;
    
    std::vector<AstNodeInfo> defs;
    if (nodeKind == model::CppAstNode::SymbolType::Macro)
    {
      AstNodeInfo nodeInfo;
      getAstNodeInfo(nodeInfo, astNodeId);
      defs = {nodeInfo};
    }
    else
      defs = helper.getDefinition(astNodeId);
    
    for (std::size_t i = 0; i < defs.size(); ++i)
    {
      const AstNodeInfo& def = defs[i];
      
      std::string filename;
      core::MenuType tmpMenuType;
      
      if (defs.size() > 1)
      {
        model::FileId fid = stoull(def.range.file.fid);
        model::File file;

        std::shared_ptr<odb::database> db = helper.getDatabase();
        util::OdbTransaction(helper.getDatabase())([&, this]() {
          db->load<model::File>(fid, file);
          filename = std::to_string(i + 1) + ". " + file.filename;
        });
      }
      
      tmpMenuType.astNodeId = def.astNodeId;
      
      //--- Jump to definition ---//
      
      tmpMenuType.category = core::Category::jumpToDef;
      tmpMenuType.name.clear();
      if (defs.size() > 1) tmpMenuType.name.push_back(filename);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Jump to definition"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "ctrl - click";
      _return.push_back(tmpMenuType);
      
      //--- Info Tree ---//

      tmpMenuType.category = core::Category::infoTree;
      tmpMenuType.name.clear();
      if (defs.size() > 1) tmpMenuType.name.push_back(filename);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Info Tree"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);
      
      //--- Info Box ---//

      tmpMenuType.category = core::Category::infoBox;
      tmpMenuType.name.clear();
      if (defs.size() > 1) tmpMenuType.name.push_back(filename);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Info Box"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "ctrl - rightclick";
      _return.push_back(tmpMenuType);

      //--- References ---//

      tmpMenuType.category = core::Category::references;
      tmpMenuType.name.clear();
      if (defs.size() > 1) tmpMenuType.name.push_back(filename);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"References", "Get definition"});
      tmpMenuType.menuItemId = RefTypes::GetDef;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      tmpMenuType.category = core::Category::pagingResult;
      tmpMenuType.name.clear();
      if (defs.size() > 1) tmpMenuType.name.push_back(filename);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"References", "Get usage"});
      tmpMenuType.menuItemId = RefTypes::GetUsage;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      if (nodeKind == model::CppAstNode::SymbolType::Variable)
      {
        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "Pointer Aliases Diagram"});
        tmpMenuType.menuItemId = core::DiagramId::POINTER_ANALYSIS;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);
      }
      if (nodeKind == model::CppAstNode::SymbolType::Function)
      {
        //--- References ---//

        tmpMenuType.category = core::Category::references;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"References", "Get Called Functions"});
        tmpMenuType.menuItemId = RefTypes::GetFuncCalls;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

        tmpMenuType.category = core::Category::pagingResult;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"References", "Get Calls of This"});
        tmpMenuType.menuItemId = RefTypes::GetCallerFuncs;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

        tmpMenuType.category = core::Category::pagingResult;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"References", "Assigns to Function Pointers"});
        tmpMenuType.menuItemId = RefTypes::GetAssignsToFuncPtrs;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

        //--- Diagrams ---//

        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "Function Call Diagram"});
        tmpMenuType.menuItemId = core::DiagramId::FUNCTION_CALL;
        tmpMenuType.helpText = "This diagram shows the called and caller functions,"
                               " and the virtual overloads";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

//        tmpMenuType.category = core::Category::diagram;
//        tmpMenuType.name.clear();
//        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
//        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "Call Path Diagram", "To this node"});
//        tmpMenuType.menuItemId = core::DiagramId::FUNCTION_PATH;
//        tmpMenuType.helpText = "This diagram shows a call path between two functions";
//        tmpMenuType.shortcut = "";
//        _return.push_back(tmpMenuType);
      }

//      if (nodeKind == model::CppAstNode::SymbolType::Variable)
//      {
//        //--- Slicing ---//
//
//        tmpMenuType.category = core::Category::slicing;
//        tmpMenuType.name.clear();
//        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
//        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Data flow analysis", "Backward"});
//        tmpMenuType.menuItemId = SlicingTypes::Before;
//        tmpMenuType.helpText = "The variable depends on these statements";
//        tmpMenuType.shortcut = "";
//        _return.push_back(tmpMenuType);
//
//        tmpMenuType.category = core::Category::slicing;
//        tmpMenuType.name.clear();
//        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
//        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Data flow analysis", "Forward"});
//        tmpMenuType.menuItemId = SlicingTypes::After;
//        tmpMenuType.helpText = "Dependencies of this variable";
//        tmpMenuType.shortcut = "";
//        _return.push_back(tmpMenuType);
//      }

      if (nodeKind == model::CppAstNode::SymbolType::Type)
      {
        //--- Diagrams ---//

        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "Detailed UML Class Diagram"});
        tmpMenuType.menuItemId = core::DiagramId::CLASS;
        tmpMenuType.helpText = "Simple class diagram showing the neighboring inheritances";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "UML Class Overview Diagram"});
        tmpMenuType.menuItemId = core::DiagramId::FULL_CLASS;
        tmpMenuType.helpText = "Shows the classes from which this class inherits"
                               " and which inherit this one";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);

        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "Class Collaboration Diagram"});
        tmpMenuType.menuItemId = core::DiagramId::CLASS_COLLABORATION;
        tmpMenuType.helpText = "Shows the individual class members and their"
                               " inheritance hierarchy";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);
      }

      //--- CODE BITES ---//

      if (nodeKind == model::CppAstNode::SymbolType::Function
       || nodeKind == model::CppAstNode::SymbolType::Macro
       || nodeKind == model::CppAstNode::SymbolType::Type)
      {
        tmpMenuType.category = core::Category::codebites;
        tmpMenuType.name.clear();
        if (defs.size() > 1) tmpMenuType.name.push_back(filename);
        tmpMenuType.name.insert(tmpMenuType.name.end(), {"Diagrams", "CodeBites"});
        tmpMenuType.menuItemId = core::DiagramId::CODE_BITES;
        tmpMenuType.helpText = "In this diagram new nodes can be opened by clicking"
                               " on a language element";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);
      }
    }
  }
  
  void CppServiceHandler::getFileMenuTypes(std::vector< core::MenuType>& _return,
    const core::FileId& fileId)
  {
    SLog() << "get file menu types called";
    
    core::MenuType tmpMenuType;
    
    //--- Diagrams ---//
    
//    tmpMenuType.category = core::Category::diagram;
//    tmpMenuType.name = {"Diagrams", "Include Dependency"};
//    tmpMenuType.menuItemId = core::DiagramId::INCLUDE_DEPENDENCY;
//    tmpMenuType.helpText = "";
//    _return.push_back(tmpMenuType);
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "UML Class Overview Diagram"};
    tmpMenuType.menuItemId = core::DiagramId::DIR_FULL_CLASS;
    tmpMenuType.helpText = "";
    _return.push_back(tmpMenuType);
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "Interface Diagram"};
    tmpMenuType.menuItemId = core::DiagramId::INTERFACE;
    tmpMenuType.helpText = "";
    _return.push_back(tmpMenuType);
    
//    tmpMenuType.category = core::Category::diagram;
//    tmpMenuType.name = {"Diagrams", "Used Components"};
//    tmpMenuType.menuItemId = core::DiagramId::COMPONENTS_USED;
//    tmpMenuType.helpText = "";
//    _return.push_back(tmpMenuType);
    
    tmpMenuType.category = core::Category::diagram; 
    tmpMenuType.name = {"Diagrams", "Component Users"}; 
    tmpMenuType.menuItemId = core::DiagramId::COMPONENT_USERS;
    tmpMenuType.helpText = "";
    _return.push_back(tmpMenuType);

    tmpMenuType.category = core::Category::infoTree;
    tmpMenuType.name = {"Info Tree"};
    tmpMenuType.menuItemId = 0; //this value has no relevance in case of an info tree, since there is only one of that
    tmpMenuType.helpText = "";
    _return.push_back(tmpMenuType);

  }
  
  void CppServiceHandler::getDirMenuTypes(std::vector< core::MenuType>& _return,
    const core::FileId& dirId)
  {
    SLog() << "get directory menu types called";
    
    model::FileId mDirId = stoull(dirId.fid);
    model::File dir;
    
    std::shared_ptr<odb::database> db = helper.getDatabase();
    util::OdbTransaction transaction = helper.getDatabase();
    transaction([&, this]() {
      db->load<model::File>(mDirId, dir);
    });
    
    core::MenuType tmpMenuType;
    
    //--- Diagrams ---//
    
    /* Internal architecture diagram is enough
    *
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "Internal Implement relationships of " + dir.filename};
    tmpMenuType.menuItemId = core::DiagramId::SUBSYSTEM_IMPLEMENT;
    _return.push_back(tmpMenuType);
    */
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "UML Class Overview Diagram"};
    tmpMenuType.menuItemId = core::DiagramId::DIR_FULL_CLASS;
    _return.push_back(tmpMenuType);
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "Internal architecture of this module"};
    tmpMenuType.menuItemId = core::DiagramId::SUBSYSTEM_DEPENDENCY;
    _return.push_back(tmpMenuType);
    
    /* Internal architecture diagram is enough
    *
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "External Implement relationships of " + dir.filename};
    tmpMenuType.menuItemId = core::DiagramId::EXTERNAL_IMPLEMENTS;
    _return.push_back(tmpMenuType);
    */
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "This module depends on..."};
    tmpMenuType.menuItemId = core::DiagramId::EXTERNAL_DEPENDENCIES;
    _return.push_back(tmpMenuType);
    
    tmpMenuType.category = core::Category::diagram;
    tmpMenuType.name = {"Diagrams", "Users of this module"};
    tmpMenuType.menuItemId = core::DiagramId::EXTERNAL_USERS;
    _return.push_back(tmpMenuType);
  }

  /*void CppServiceHandler::getFileDiagramTypes(
    std::vector< core::DiagramType> & _return,
    const core::FileId& fileId)
  {
    // Your implementation goes here
    printf("getFileDiagramTypes\n");
  }*/
  
  void CppServiceHandler::getFileDiagram(std::string& _return,
    const core::FileId& fileId, const core::DiagramId::type diagramId)
  {
    _return = helper.getFileDiagram(fileId, diagramId);
  }
  
  /*void CppServiceHandler::getFileReferenceTypes(
    std::vector< core::ReferenceType> & _return,
    const core::FileId& fileId)
  {
    // Your implementation goes here
    printf("getFileReferenceTypes\n");
  }*/
  
  void CppServiceHandler::getFileReferences(std::vector<AstNodeInfo> & _return,
    const core::FileId& fileId, const RefTypes::type referenceId)
  {
    // Your implementation goes here
    printf("getFileReferences\n");
  }

  void CppServiceHandler::getTypeDefinitions(
    std::vector<core::AstNodeId> & _return,
    const std::string& path)
  {
    _return = helper.getTypeDefinitions(path);
  }

  void CppServiceHandler::getInfoTree(std::vector<InfoNode> & _return,
    const core::AstNodeId& astNodeId)
  {  
    _return = helper.getInfoTree(astNodeId);
  }
  
  void CppServiceHandler::getSubInfoTree(std::vector<InfoNode> & _return,
      const core::AstNodeId& astNodeId, const InfoQuery& query)
  {
    _return = helper.getSubInfoTree(astNodeId, query);
  }

  void CppServiceHandler::getCatalogue(std::vector<InfoNode> & _return)
  {
    _return = helper.getCatalogue();
  }

  void CppServiceHandler::getSubCatalogue(std::vector<InfoNode> & _return,
   const InfoQuery& query)
  {
    _return = helper.getSubCatalogue(query);
  }

  void CppServiceHandler::getInfoTreeForFile(
  std::vector<InfoNode>& _return,
  const core::FileId& fileId)
  {
    _return = helper.getInfoTreeForFile(fileId);
  }

  void CppServiceHandler::getSubInfoTreeForFile(
      std::vector<InfoNode>& _return,
      const core::FileId& fileId,
      const InfoQuery& query)
  {
    _return = helper.getSubInfoTreeForFile(fileId, query);
  }

  core::AstNodeId CppServiceHandler::makeAstNodeId( int nodeId_)
  {
    core::AstNodeId id;
    id.astNodeId = std::to_string(nodeId_);
    return id;
  }
  
  void CppServiceHandler::getBackwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos)
  {   
    // PLEASE DO NOT COMMENT OUT!!!!! (INSTANT SEGFAULT ON RIGHT CLICK) 
    helper.getBackwardSlicePos(_return, filePos);
  }
  
  void CppServiceHandler::getForwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos)
  {
    // PLEASE DO NOT COMMENT OUT!!!!! (INSTANT SEGFAULT ON RIGHT CLICK)
    helper.getForwardSlicePos(_return, filePos);    
  }  
  
  void CppServiceHandler::getSyntaxHighlight(
    std::vector<core::SyntaxHighlight>& _return,
    const core::FileId& fileId)
  {
    _return = helper.getSyntaxHighlight(fileId);
  }
  
} // language
} // service
} // cc

