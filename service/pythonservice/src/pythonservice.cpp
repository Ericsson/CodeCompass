#include <pythonservice/pythonservice.h>

#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <set>

#include <odb/query.hxx>
#include <odb/query-dynamic.hxx>
#include <odb/result.hxx>

#include <util/streamlog.h>
#include <langservicelib/utils.h>
#include <langservicelib/odbquery.h>
#include <plugin/servicenotavailexception.h>

#include <model/python/pythonastnode.h>
#include <model/python/pythonbinding.h>

//#include "treehandler.cpp"

#include <chrono>

namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{


PythonServiceHandler::PythonServiceHandler(std::shared_ptr<odb::database> db_)
  : db(db_)
  , transaction(db_)
  , helper(db_)
{ }


// Complusory implementation of pure virtual methods provied by LanguageServiceIf:
//

void PythonServiceHandler::getAstNodeInfoByPosition(AstNodeInfo& _return,
  const ::cc::service::core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  transaction([&, this]()
  {
    _return = helper.createAstNodeInfo(helper.queryAstNodeByPosition(fpos));
  });
}

void PythonServiceHandler::getInfoBox(InfoBox& _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
  transaction([&, this]()
  {
    auto binding = *(db->load<model::PythonBinding>(
      std::stoull(astNodeId.astNodeId)));

    if(binding.location.file.load()->content)
    {
      const auto& textSrc = textRange(
        binding.location.file->content.load()->content, binding.location.range);      

      _return.information += "<" + binding.type + ">\n" + textSrc;
    }

    _return.documentation = binding.documentation;

    _return.fileType = core::FileType::PythonScript;
  });
}

void PythonServiceHandler::getInfoBoxByPosition(InfoBox& _return,
  const ::cc::service::core::FilePosition& fpos,
  const std::vector<std::string> & filters)
{
  transaction([&, this]()
  {
    auto bindings = helper.queryBindingByAstNode(
      helper.queryAstNodeByPosition(fpos));

    for(auto& binding : bindings)
    {
      if(binding.location.file.load()->content)
      {
        const auto& textSrc = textRange(
          binding.location.file->content.load()->content, binding.location.range);      

        _return.information += "<" + binding.type + ">\n" + textSrc;
      }

      _return.documentation = binding.documentation;
    }

    _return.fileType = core::FileType::PythonScript;
  });
}

void PythonServiceHandler::getAstNodeInfo(AstNodeInfo& _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
  transaction([&, this]()
  {
    _return = helper.createAstNodeInfoByAstNodeId(
      std::stoull(astNodeId.astNodeId));
  });
}

void PythonServiceHandler::getDiagram(std::string& _return,
  const ::cc::service::core::AstNodeId& astNodeId,
  const ::cc::service::core::DiagramId::type diagramId)
{
  transaction([&, this]()
  {
    auto node = *db->load<model::PythonAstNode>(std::stoull(astNodeId.astNodeId));

    switch(diagramId)
    {
      case ::cc::service::core::DiagramId::type::FUNCTION_CALL:
        _return = helper.getFunctionCallDiagram(node);
        return;

      default:
        SLog(util::DEBUG) <<
          "Unexpected diagram type in PythonServiceHandler::getDiagram.";
    }

    _return = "";
  });
}

void PythonServiceHandler::getLegend(std::string& _return,
  const ::cc::service::core::DiagramId::type diagramId)
{
  _return = "";
}

void PythonServiceHandler::getFunctionCallPathDiagram(std::string& _return,
  const ::cc::service::core::AstNodeId& astNodeIdFrom,
  const ::cc::service::core::AstNodeId& astNodeIdTo)
{
}

void PythonServiceHandler::getReferences(std::vector<AstNodeInfo> & _return,
  const ::cc::service::core::AstNodeId& astNodeId,
  const RefTypes::type referenceId)
{
  transaction([&, this]()
  {
    auto astNode = db->load<model::PythonAstNode>(std::stoull(
      astNodeId.astNodeId));
    if (astNode->ast_type == model::PythonAstNode::AstType::Attribute)
    {
      getVariableReferences(_return, *astNode, referenceId);

      if(!_return.empty())
        return;
    }

    switch(referenceId)
    {
      case RefTypes::GetDef:
      {
        if(astNode->base_binding.null())
        {
          for(auto& binding : helper.queryBindingByAstNode(astNode->id))
            _return.push_back(helper.createAstNodeInfo(binding));

          //for(auto& pdef : helper.queryPossibleDefs(*astNode))
          //  _return.push_back(helper.createAstNodeInfo(pdef));
        }
        else
          _return.push_back(helper.createAstNodeInfoByBindingId(
            *(astNode->base_binding)));
        break;
      }

      case RefTypes::GetUsage:
      {
        for(auto& usage : helper.queryUsageByAstNode(
          std::stoull(astNodeId.astNodeId)))
        {
          _return.push_back(helper.createAstNodeInfo(usage));
        }

        //for(auto& usage : helper.queryPossibleUsages(*astNode))
        //  _return.push_back(helper.createAstNodeInfo(usage));
        break;
      }

      default:
        SLog(util::DEBUG) <<
          "Unexpected RefTypes in PythonServiceHandler::getReferences.";
        break;
    }
  });
}

void PythonServiceHandler::getReferencesInFile(std::vector<AstNodeInfo> & _return,
  const ::cc::service::core::AstNodeId& astNodeId,
  const RefTypes::type referenceId,
  const ::cc::service::core::FileId& fileId)
{
  using namespace model;

  transaction([&, this]()
  {
    model::PythonAstNode::pktype astId = std::stoull(astNodeId.astNodeId);
    model::FileId fid = std::stoull(fileId.fid);

    auto node = *(db->load<model::PythonAstNode>(astId));
    if (node.ast_type == model::PythonAstNode::AstType::Attribute)
    {
      getVariableReferences(_return, node, referenceId, &fid);

      if(!_return.empty())
        return;
    }

    switch(referenceId)
    {
      case RefTypes::GetUsage:
      {
        auto usages = helper.queryUsageByAstNode(astId);
        for(auto& usage : usages)
        {
          if(usage.location.file.object_id() == fid)
            _return.push_back(helper.createAstNodeInfo(usage));
        }

        // Add the node itself, if it has not already added.
        auto itself = std::find_if(_return.begin(), _return.end(),
          [&astNodeId](const AstNodeInfo& nodeInfo)
          {
            return nodeInfo.astNodeId.astNodeId == astNodeId.astNodeId;
          });
        if(itself == _return.end())
        {
          _return.push_back(
            helper.createAstNodeInfo(*db->load<model::PythonAstNode>(astId)));
        }

        //if(node.ast_type != model::PythonAstNode::AstType::Variable)
        //{
        //  for(auto& usage : helper.queryPossibleUsages(node))
        //  {
        //    if(usage.location.file.object_id() == fid)
        //      _return.push_back(helper.createAstNodeInfo(usage));
        //  }
        //}
        break;
      }
      
      default:
        SLog(util::DEBUG) <<
          "Unexpected RefTypes in PythonServiceHandler::getReferencesInFile.";
        break;
    }
  });
}

void PythonServiceHandler::getPage(::cc::service::core::RangedHitCountResult& _return,
  const ::cc::service::core::AstNodeId& astNodeId,
  const RefTypes::type referenceId,
  const int32_t pageSize,
  const int32_t pageNo)
{
  transaction([&, this]()
  {
    auto node = db->load<model::PythonAstNode>(
      std::stoull(astNodeId.astNodeId));
    if (node->ast_type == model::PythonAstNode::AstType::Attribute)
    {
      getVariableReferencesPage(_return, *node, referenceId, pageSize, pageNo);
      return;
    }

    switch(referenceId)
    {
      case RefTypes::GetUsage:
        _return = helper.getReferencesPage(astNodeId, pageSize, pageNo);
        break;

      default:
        SLog(util::DEBUG) <<
          "Unexpected RefTypes in PythonServiceHandler::getPage.";
        break;
    }
  });
}

void PythonServiceHandler::getFileDiagram(std::string& _return,
  const ::cc::service::core::FileId& fileId,
  const ::cc::service::core::DiagramId::type diagramId)
{
}

void PythonServiceHandler::getFileReferences(std::vector<AstNodeInfo> & _return,
  const ::cc::service::core::FileId& fileId,
  const RefTypes::type referenceId)
{
}

void PythonServiceHandler::getInfoTree(std::vector<InfoNode> & _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
  transaction([&, this]()
  {
    auto astNode = db->load<model::PythonAstNode>(
      std::stoull(astNodeId.astNodeId));
    auto handler = TreeHandler::getHandler(db, astNode->ast_type);

    _return = handler->getInfoTree(*astNode);
  });
}

void PythonServiceHandler::getSubInfoTree(std::vector<InfoNode> & _return,
  const ::cc::service::core::AstNodeId& astNodeId,
  const InfoQuery& query)
{
  transaction([&, this]()
  {
    auto astNode = db->load<model::PythonAstNode>(
      std::stoull(astNodeId.astNodeId));
    auto handler = TreeHandler::getHandler(db, astNode->ast_type);

    _return = handler->getSubInfoTree(*astNode, query);
  });
}

void PythonServiceHandler::getCatalogue(std::vector<InfoNode> & _return)
{
}

void PythonServiceHandler::getSubCatalogue(std::vector<InfoNode> & _return,
  const InfoQuery& query)
{
}

void PythonServiceHandler::getInfoTreeForFile(std::vector<InfoNode> & _return,
  const ::cc::service::core::FileId& fileId)
{
  transaction([&, this]()
  {
    FileTreeHandler handler(db);

    _return = handler.getInfoTreeForFile(stoull(fileId.fid));
  });
}

void PythonServiceHandler::getSubInfoTreeForFile(std::vector<InfoNode> & _return,
  const ::cc::service::core::FileId& fileId,
  const InfoQuery& query)
{
  transaction([&, this]()
  {
    FileTreeHandler handler(db);

    _return = handler.getSubInfoTreeForFile(stoull(fileId.fid), query);
  });
}

void PythonServiceHandler::getMenuTypes(
  std::vector<::cc::service::core::MenuType> & _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
  transaction([&, this]
  {
      using namespace model;
    core::MenuType tmpMenuType;

    //
    // Resolved menu

    /**
     * Creates a menus for a definition
     */
    auto createDefinitionMenus = [&_return, &tmpMenuType](
      const std::string& defText_,
      const std::string& astNodeId_,
      const model::PythonBinding::Kind kind)
    {
      tmpMenuType.astNodeId.astNodeId = astNodeId_;

      //--- jump to definition ---//

      tmpMenuType.category = core::Category::jumpToDef;
      tmpMenuType.name.clear();
      if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Jump to definition"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "ctrl - click";
      _return.push_back(tmpMenuType);

      //--- Info Tree ---//

      tmpMenuType.category = core::Category::infoTree;
      tmpMenuType.name.clear();
      if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Info Tree"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      //--- Info Box ---//

      tmpMenuType.category = core::Category::infoBox;
      tmpMenuType.name.clear();
      if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Info Box"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "ctrl - rightclick";
      _return.push_back(tmpMenuType);

      //--- Diagrams ---//

      if(kind == model::PythonBinding::Kind::Function ||
        kind == model::PythonBinding::Kind::Method)
      {
        tmpMenuType.category = core::Category::diagram;
        tmpMenuType.name.clear();
        if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
        tmpMenuType.name.insert(tmpMenuType.name.end(),
         {"Diagrams", "Function call diagram"});
        tmpMenuType.menuItemId = core::DiagramId::FUNCTION_CALL;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);
      }

      //--- References ---//

      tmpMenuType.category = core::Category::references;
      tmpMenuType.name.clear();
      if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
      tmpMenuType.name.insert(tmpMenuType.name.end(),
        {"References", "Get resolved definition"});
      tmpMenuType.menuItemId = RefTypes::GetDef;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      tmpMenuType.category = core::Category::references;
      tmpMenuType.name.clear();
      if (!defText_.empty()) tmpMenuType.name.push_back(defText_);
      tmpMenuType.name.insert(tmpMenuType.name.end(),
        {"References", "Get resolved usages"});
      tmpMenuType.menuItemId = RefTypes::GetUsage;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      _return.push_back(tmpMenuType);

      /*tmpMenuType.category = core::Category::pagingResult;
      tmpMenuType.name = {"References", "Get usage"};
      tmpMenuType.menuItemId = RefTypes::GetUsage;
      tmpMenuType.helpText = "";
      _return.push_back(tmpMenuType);*/
    };

    auto astNode = *(db->load<PythonAstNode>(
      std::stoull(astNodeId.astNodeId)));

    std::vector<PythonBinding> bindings;
    
    if (astNode.ast_type == PythonAstNode::AstType::Attribute)
    {
      // Get bindings by VariableRef table
        
      auto defs = helper.getVarRefsForAstNode(astNode.id,
        queryVariableRef::refType == PythonVariableRef::RefType::Definition);
      
      if(defs.empty())
      { // attribute not in pythonvariableref table - possibly an error, like "os.path"
        bindings = helper.queryBindingByAstNode(astNode.id); // load binding in normal way
      }
      else
      {
        for (const auto& def : defs)
        {
          auto binding = db->find<PythonBinding>(def.astNode.object_id());
          if (!binding)
          {
            SLog(util::ERROR)
              << "No binding for definition: " << def.astNode.object_id();
            continue;
          }
          
          bindings.emplace_back(*binding);
        }
      }
    }
    else if(astNode.ast_type == PythonAstNode::AstType::Variable)
    {
      bindings = helper.queryBindingByAstNode(astNode.id);

      auto min_binding = std::min_element(bindings.begin(), bindings.end(), 
        [](const PythonBinding& lhs, const PythonBinding& rhs)
        {
          return 
            lhs.location.range.start.line 
            < 
            rhs.location.range.start.line
            ||
            (
              lhs.location.range.start.line 
              == 
              rhs.location.range.start.line
            &&
              lhs.location.range.start.column 
              < 
              rhs.location.range.start.column
            );
        });

        if(min_binding != bindings.end())
        {
          auto from_remove = std::remove_if(bindings.begin(), bindings.end(), 
            [&min_binding](const PythonBinding& b)
            {
              return b.id != min_binding->id;
            });
          bindings.erase(from_remove, bindings.end());
        }
    }
    else if(!astNode.base_binding.null())
    {
      bindings = { *db->load<PythonBinding>(*astNode.base_binding) };
    }
    else 
    { // Simply load bindings
      bindings = helper.queryBindingByAstNode(astNode.id);
    }

    // Add definition menus
    for (const auto& binding : bindings)
    {
      std::string defText;
      if (bindings.size() > 1)
      {
        defText = helper.getFormattedQName(binding);
      }

      createDefinitionMenus(defText, std::to_string(binding.id), binding.kind);
    }

    //
    // Common menu
    //--- Possible other definitions ---//

    if(astNode.base_binding.null() && astNode.name != "self")
    {
      std::vector<core::MenuType> all_pdef;
      for(auto& pdef : helper.queryPossibleDefs(
        std::stoull(astNodeId.astNodeId), bindings, true))
      {
        tmpMenuType.astNodeId.astNodeId = std::to_string(pdef.id);
        auto binding = *(db->load<PythonBinding>(pdef.id));
        std::string defText = helper.getFormattedQName(binding);
        tmpMenuType.category = core::Category::jumpToDef;
        tmpMenuType.name = {"Possible other definitions"};
        tmpMenuType.name.push_back(defText);
        tmpMenuType.menuItemId = 0;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        all_pdef.push_back(tmpMenuType);
      }
      std::sort(all_pdef.begin(), all_pdef.end(),
        [](const core::MenuType& lhs, const core::MenuType& rhs)
        {
          return lhs.name.back() < rhs.name.back();
        });
      int i = 0;
      auto all_pdef_it = all_pdef.begin();
      while(i < 10 && all_pdef_it != all_pdef.end())
      {
        _return.push_back(*all_pdef_it);
        ++i, ++all_pdef_it;
      }
      if(all_pdef.size() > 10)
      {
        tmpMenuType.astNodeId.astNodeId = std::to_string(astNode.id);

        tmpMenuType.category = core::Category::references;
        tmpMenuType.name = {"Possible other definitions"};
        tmpMenuType.name.push_back("more...");
        tmpMenuType.menuItemId = RefTypes::GetDef;
        tmpMenuType.helpText = "";
        tmpMenuType.shortcut = "";
        _return.push_back(tmpMenuType);
      }
    }


    //
    // Unresolved menu
    if(bindings.empty())
    {
      //--- Info Box ---//
      
      tmpMenuType.astNodeId.astNodeId = astNodeId.astNodeId;

      tmpMenuType.category = core::Category::infoTree;
      tmpMenuType.name.clear();
      tmpMenuType.name.insert(tmpMenuType.name.end(), {"Info Tree"});
      tmpMenuType.menuItemId = 0;
      tmpMenuType.helpText = "";
      tmpMenuType.shortcut = "";
      tmpMenuType.astNodeId.astNodeId = astNodeId.astNodeId;
      _return.push_back(tmpMenuType);
    }
  });
}

void PythonServiceHandler::getFileMenuTypes(std::vector<::cc::service::core::MenuType> & _return,
  const ::cc::service::core::FileId& fileId)
{
}

void PythonServiceHandler::getDirMenuTypes(std::vector<::cc::service::core::MenuType> & _return,
  const ::cc::service::core::FileId& dirId)
{
}

void PythonServiceHandler::getSourceCode(std::string& _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
}

void PythonServiceHandler::getDocComment(std::string& _return,
  const ::cc::service::core::AstNodeId& astNodeId)
{
}

void PythonServiceHandler::getBackwardSlicePos(std::vector<::cc::service::core::Range> & _return,
  const ::cc::service::core::FilePosition& filePos)
{
}

void PythonServiceHandler::getForwardSlicePos(std::vector<::cc::service::core::Range> & _return,
  const ::cc::service::core::FilePosition& filePos)
{
}

void PythonServiceHandler::getTypeDefinitions(
  std::vector<::cc::service::core::AstNodeId> & _return,
  const std::string& path)
{
}

void PythonServiceHandler::getVariableReferences(
  std::vector<AstNodeInfo> & _return,
  const model::PythonAstNode& astNode_,
  const RefTypes::type referenceId_,
  const model::FileId* const fileId_)
{
  if (referenceId_ != RefTypes::GetDef && referenceId_ != RefTypes::GetUsage)
  {
    SLog(util::DEBUG) << "Unexpected reference type: " << referenceId_;
    return;
  }

  getResolvedVariableReferences(_return, astNode_, referenceId_, fileId_);
  // if (referenceId_ == RefTypes::GetDef && _return.empty())
  // {
  //   // We didn't find any resolved definition so have to find some possible ones

  //   auto pdefs = helper.queryPossibleDefs(astNode_);
  //   for(auto& pdef : pdefs)
  //   {
  //     _return.push_back(helper.createAstNodeInfo(pdef));
  //   }
  // }

  // Sort by range and remove duplicates
  std::sort(_return.begin(), _return.end(),
    [](const AstNodeInfo& left, const AstNodeInfo& right)
    {
      return
        left.range.file.fid < right.range.file.fid || (
          left.range.file.fid == right.range.file.fid &&
          left.range.range.startpos.line < right.range.range.startpos.line
        );
    });
}

void PythonServiceHandler::getResolvedVariableReferences(
  std::vector<AstNodeInfo> & _return,
  const model::PythonAstNode& astNode_,
  const RefTypes::type referenceId_,
  const model::FileId* const fileId_)
{
  if (referenceId_ != RefTypes::GetDef && referenceId_ != RefTypes::GetUsage)
  {
    SLog(util::DEBUG) << "Unexpected reference type: " << referenceId_;
    return;
  }

  // Get references by mangled name
  queryVariableRef refQuery;
  if (referenceId_ == RefTypes::GetDef)
  {
    // We need definitions only
    refQuery = queryVariableRef::refType ==
      model::PythonVariableRef::RefType::Definition;
  }

  auto refs = helper.getVarRefsForAstNode(astNode_.id, refQuery);
  for (const auto& ref : refs)
  {
    auto defAstNode = ref.astNode.load();
    if (!fileId_ || defAstNode->location.file.object_id() == *fileId_)
    {
      // No file filter or matching file
      _return.push_back(helper.createAstNodeInfo(*defAstNode));
    }
  }
}

void PythonServiceHandler::getVariableReferencesPage(
  ::cc::service::core::RangedHitCountResult& _return,
  const model::PythonAstNode& astNode_,
  const RefTypes::type referenceId_,
  const int32_t pageSize_,
  const int32_t pageNo_)
{
  using queryVariableRef = odb::query<model::PythonVariableRef>;
  using queryFileId = odb::query<model::PythonVariableRefFile>;

  if (referenceId_ != RefTypes::GetDef && referenceId_ != RefTypes::GetUsage)
  {
    SLog(util::DEBUG) << "Unexpected reference type: " << referenceId_;
    return;
  }

  // Get mangled name by AstNode
  auto mangledName = helper.getVarRefMangledName(astNode_.id);
  if (mangledName.empty())
  {
    return;
  }

  // Get files by mangled name
  queryFileId refQuery(queryFileId::Ref::mangledName == mangledName);
  if (referenceId_ == RefTypes::GetDef)
  {
    // We need definitions only
    refQuery = refQuery && (queryVariableRef::refType ==
      model::PythonVariableRef::RefType::Definition);
  }

  // Do the paging stuff
  /* TODO: There is a design issue with the LanguageService so we can't write
   * fast queries (see artf518144 for details)
    "LIMIT " + std::to_string(pageSize_) +
    "OFFSET " + std::to_string(pageSize_ * pageNo_);*/

  const int32_t startCount = pageSize_ * pageNo_;
  const int32_t endCount = pageSize_ * (pageNo_ + 1);
  int32_t totalCount = 0;

  auto files = db->query(refQuery);
  for (const auto& fref : files)
  {
    if (totalCount >= startCount && totalCount < endCount)
    {
      core::HitCountResult hitcount;
      hitcount.finfo.file.fid = std::to_string(fref.file);
      hitcount.finfo.name = fref.filename;
      hitcount.finfo.path = fref.path;

      _return.results.push_back(hitcount);
    }

    ++totalCount;
  }

  _return.firstFileIndex = startCount;
  _return.totalFiles = totalCount;
}

void PythonServiceHandler::getSyntaxHighlight(
    std::vector< ::cc::service::core::SyntaxHighlight> & _return,
    const  ::cc::service::core::FileId& fileId)
{
}

} // python
} // language
} // service
} // cc
