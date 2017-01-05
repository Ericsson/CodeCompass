#include <pythonservice/treehandler.h>


#include <boost/algorithm/string_regex.hpp>
#include <odb/query.hxx>

#include <util/streamlog.h>
#include <model/filecontent.h>

#include <model/python/pythonvariableref-odb.hxx>

namespace cc
{
namespace service
{ 
namespace language
{
namespace python
{


// DefaultHandler
//

class DefaultHandler : public TreeHandler
{
public:
  DefaultHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
    std::vector<InfoNode> ret;

    ret.push_back(makeInfoNode({}, "Name", astNode_.name,
      helper.createAstNodeInfo(astNode_)));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(const model::PythonAstNode& astNode_,
    const InfoQuery& query) const override
  {
    return {};
  }
}; // DefaultHandler


// FunctionHandler
//

class FunctionHandler : public TreeHandler
{
  enum class SubQuery
  {
    Decorators = 1,
    Parameters,
    Overrides,
    OverridenBy,
    CalledBy,
    DeferedCalledBy,
    Calls,
    Locals
  };

public:
  FunctionHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
    std::vector<InfoNode> ret;

    auto binding = *(db->load<model::PythonBinding>(astNode_.id));

    // short name
    ret.push_back(makeInfoNode({}, "Name", astNode_.name,
      helper.createAstNodeInfo(binding)));

    // type
    ret.push_back(makeInfoNode({}, "Type", binding.type));

    // declared
    ret.push_back(makeInfoNode({}, "Defined", getFileLoc(binding),
      helper.createAstNodeInfo(binding)));

    // defered queries
    ret.push_back(makeInfoQueryNode({"Decorators"}, (int)SubQuery::Decorators));
    ret.push_back(makeInfoQueryNode({"Parameters"}, (int)SubQuery::Parameters));
    ret.push_back(makeInfoQueryNode({"Overriddes"}, (int)SubQuery::Overrides));
    ret.push_back(makeInfoQueryNode({"Overridden by"}, (int)SubQuery::OverridenBy));
    ret.push_back(makeInfoQueryNode({"Called by"}, (int)SubQuery::CalledBy));
    ret.push_back(makeInfoQueryNode({"Calls"}, (int)SubQuery::Calls));
    ret.push_back(makeInfoQueryNode({"Local variables"}, (int)SubQuery::Locals));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(
    const model::PythonAstNode& astNode_,
    const InfoQuery& infoQuery) const override
  {
    using namespace model;

    auto binding = *(db->load<PythonBinding>(astNode_.id));

    
    auto decorators = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto decorators = db->query<PythonDecorator>(
        queryDecorator::target == binding.id);
      
      for(auto& decorator : decorators)
        ret.push_back(makeInfoNode({}, "", decorator.value));

      return ret;
    };

    auto parameters = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto parameters = db->query<PythonFunctionParam>(
        queryParameter::target == binding.id);
      
      for(auto& param : parameters)
      {
        auto param_node = *(db->load<PythonAstNode>(param.id));
        ret.push_back(makeInfoNode({}, getFileLoc(param_node), param_node.name,
          helper.createAstNodeInfo(param_node)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto overrides = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      if(astNode_.container_binding.null())
        throw std::runtime_error("PythonAstNode has no container_binding.");

      auto bases = db->query<PythonInheritance>(
        queryInheritance::target == *(astNode_.container_binding)
        &&
        queryInheritance::kind != PythonInheritance::Missing);

      std::vector<PythonAstNode::pktype> bases_id;
      for(auto& inheritence_base : bases)
        bases_id.push_back(inheritence_base.base);

      if(!bases_id.empty()) // it is necessarily because of odb::in_range
      {
        auto overrides = db->query<PythonAstNode>(
          queryAstNode::name == astNode_.name
          &&
          queryAstNode::ast_type == PythonAstNode::AstType::Function
          &&
          queryAstNode::container_binding.in_range(bases_id.begin(), bases_id.end()));
        
        for(auto& override : overrides)
        {
          auto nodei = helper.createAstNodeInfo(override);
          ret.push_back(makeInfoNode({}, getFileLoc(override),
            nodei.astNodeSrcText, nodei));
        }

        sortNodeInfoByFilePos(ret, db);
      }

      return ret;
    };

    auto overriden_by = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      if(astNode_.container_binding.null())
        throw std::runtime_error("PythonAstNode has no container_binding.");

      auto targets = db->query<PythonInheritance>(
        queryInheritance::base == *(astNode_.container_binding)
        &&
        queryInheritance::kind != PythonInheritance::Missing);

      std::vector<PythonAstNode::pktype> target_id;
      for(auto& inheritence_target : targets)
        target_id.push_back(inheritence_target.target);

      if(!target_id.empty()) // it is necessarily because of odb::in_range
      {
        auto overriden_bys = db->query<PythonAstNode>(
          queryAstNode::name == astNode_.name
          &&
          queryAstNode::ast_type == PythonAstNode::AstType::Function
          &&
          queryAstNode::container_binding.in_range(target_id.begin(), target_id.end()));
        
        for(auto& overriden_by : overriden_bys)
        {
          auto nodei = helper.createAstNodeInfo(overriden_by);
          ret.push_back(makeInfoNode({}, getFileLoc(overriden_by),
            nodei.astNodeSrcText, nodei));
        }

        sortNodeInfoByFilePos(ret, db);
      }

      return ret;
    };

    auto calledby = [this, &astNode_, &binding, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      if(infoQuery.filters.empty())
      {
        auto callers = helper.queryUsageByAstNode(astNode_);
        
        for(auto& caller : callers)
        {
          if(caller.id == binding.id || db->find<PythonFunctionDef>(caller.id))
            continue;

          auto nodei = helper.createAstNodeInfo(caller);
          ret.push_back(
            makeInfoNode({"Resolved callers"}, getFileLoc(caller),
              nodei.astNodeSrcText, nodei));
        }

        sortNodeInfoByFilePos(ret, db);
      }

      // possible callers:
      auto possible_callers = helper.queryPossibleUsages(astNode_);
      sortNodeInfoByPackagePos(
        ret, possible_callers, { "Possible callers" }, infoQuery, helper);

      return ret;
    };

    auto calls = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto calls = db->query<PythonAstNode>(
        queryAstNode::container_binding == binding.id &&
        (queryAstNode::ast_type == PythonAstNode::AstType::Call ||
        queryAstNode::ast_type == PythonAstNode::AstType::Function) &&
        queryAstNode::base_binding.is_null());
      
      for(auto& call : calls)
      {
        ret.push_back(makeInfoNode({}, getFileLoc(call), call.name,
          helper.createAstNodeInfo(call)));
      }

      // possible calls:
      auto possible_calls = db->query<PythonAstNode>(
        helper.predContainsLocationRange<queryAstNode>(
          binding.location,
          queryAstNode::location)
        &&
        (queryAstNode::ast_type == PythonAstNode::AstType::Unknown
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Bindingless));

      for(auto& possible_call : possible_calls)
      {
        // filter non Function (or Unknown) kind AstNode from Unknowns
        if(possible_call.ast_type == PythonAstNode::AstType::Unknown)
        {
          auto possible_call_node = *(db->load<PythonUnknown>(possible_call.id));
          if(possible_call_node.kind != PythonUnknown::Function ||
             possible_call_node.kind != PythonUnknown::Unknown)
            continue;
        }

        // common filters
        if(PythonQueryHelper::common_python_keywords.count(possible_call.name))
          continue;

        ret.push_back(makeInfoNode({"Possible calls"}, getFileLoc(possible_call),
          possible_call.name, helper.createAstNodeInfo(possible_call)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto locals = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto locals = db->query<PythonVariable>(
        queryVariable::target == binding.id);
      
      for(auto& local : locals)
      {
        auto local_node = *(db->load<PythonAstNode>(local.id));
        ret.push_back(makeInfoNode({}, getFileLoc(local_node), local_node.name,
          helper.createAstNodeInfo(local_node)));
      }

      // possible locals:
      auto possible_locals = db->query<PythonAstNode>(
        helper.predContainsLocationRange<queryAstNode>(
          binding.location,
          queryAstNode::location)
        &&
        (queryAstNode::ast_type == PythonAstNode::AstType::Unknown
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Bindingless));

      for(auto& possible_local : possible_locals)
      {
        // filters for Unknown
        if(possible_local.ast_type == PythonAstNode::AstType::Unknown)
        {
          auto possible_local_node = *(db->load<PythonUnknown>(possible_local.id));
          
          if(possible_local_node.kind != PythonUnknown::Attribute ||
             possible_local_node.kind != PythonUnknown::Unknown)
            continue;
        }

        // common filters
        if(PythonQueryHelper::common_python_keywords.count(possible_local.name))
          continue;

        ret.push_back(makeInfoNode({"Possible locals"}, getFileLoc(possible_local),
          possible_local.name, helper.createAstNodeInfo(possible_local)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };


    auto queryId = static_cast<SubQuery>(infoQuery.queryId);
    switch (queryId)
    {
      case SubQuery::Decorators:
        return decorators();
      case SubQuery::Parameters:
        return parameters();
      case SubQuery::Overrides:
        return overrides();
      case SubQuery::OverridenBy:
        return overriden_by();
      case SubQuery::CalledBy:
        return calledby();
      case SubQuery::Calls:
        return calls();
      case SubQuery::Locals:
        return locals();
      default:
        break;
    }

    return {};
  }
}; // FunctionHandler



// VariableHandler
//

class VariableHandler : public TreeHandler
{
  enum class SubQuery
  {
    Reads = 1,
    Writes,
    PossibleUsage
  };

public:
  VariableHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
      using namespace model;
    std::vector<InfoNode> ret;

    auto binding = *(db->load<model::PythonBinding>(astNode_.id));

    // short name
    ret.push_back(makeInfoNode({}, "Name", astNode_.name,
      helper.createAstNodeInfo(binding)));

    // type
    ret.push_back(makeInfoNode({}, "Type", binding.type));

    // declared    
    PythonBinding def_binding = binding;
    
    if (astNode_.ast_type == PythonAstNode::AstType::Attribute)
    {
      auto vref = helper.getVarRefsForAstNode(astNode_.id);
      if(!vref.empty())
      {
        auto varRef = *(vref.begin());
        auto def_result = db->query<PythonVariableRef>(
          queryVariableRef::mangledName == varRef.mangledName &&
          queryVariableRef::refType == PythonVariableRef::RefType::Definition);
        
        if(!def_result.empty())
          def_binding = *db->load<PythonBinding>(
            (*def_result.begin()).astNode.object_id());
      }
    }
    
    ret.push_back(makeInfoNode({}, "Defined", getFileLoc(binding),
      helper.createAstNodeInfo(def_binding)));
    
    

//    ret.push_back(makeInfoNode({}, "Defined", getFileLoc(binding),
//      helper.createAstNodeInfo(binding)));

    // defered queries
    ret.push_back(makeInfoQueryNode({"Reads"}, (int)SubQuery::Reads));
    ret.push_back(makeInfoQueryNode({"Writes"}, (int)SubQuery::Writes));
    ret.push_back(makeInfoQueryNode({"Possible usage"}, (int)SubQuery::PossibleUsage));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(
    const model::PythonAstNode& astNode_,
    const InfoQuery& infoQuery) const override
  {
    using namespace model;
    
    enum class VarRefType {
      WriteAndDef,
      Read
    };

    auto getVarRefs = [this, &astNode_](VarRefType type_)
    {
      using queryVariableRef = odb::query<PythonVariableRef>;

      std::vector<InfoNode> ret;

      auto mangledNameRef = db->query<model::PythonVariableRef>(
        queryVariableRef::astNode == astNode_.id);
      if (mangledNameRef.empty())
      {
        SLog(util::DEBUG)
          << "UMangled name for " << astNode_.id << " not found!";
        return ret;
      }

      // Get references by mangled name
      queryVariableRef refQuery(
        queryVariableRef::mangledName == mangledNameRef.begin()->mangledName);
      if (type_ == VarRefType::WriteAndDef)
      {
        // We need writes and definitions
        refQuery = refQuery && (
          (queryVariableRef::refType ==
           model::PythonVariableRef::RefType::Write) ||
          (queryVariableRef::refType ==
           model::PythonVariableRef::RefType::Definition));
      }
      else
      {
        // We need reads only
        refQuery = refQuery && (queryVariableRef::refType ==
          model::PythonVariableRef::RefType::Read);
      }

      auto refs = db->query(refQuery);
      for (const auto& ref : refs)
      {
        auto defAstNode = ref.astNode.load();
        auto nodei = helper.createAstNodeInfo(*defAstNode);

        ret.push_back(makeInfoNode(
          {}, getFileLoc(*defAstNode), nodei.astNodeSrcText, nodei));
      }

      sortNodeInfoByFilePos(ret, db);
      return ret;
    };


    // Original verson of reads using refs
    auto reads = [this, &astNode_]
    {
      std::vector<InfoNode> ret;

      auto binding = *(db->load<PythonBinding>(astNode_.id));
      auto usages = helper.queryUsageByAstNode(astNode_);
      for(auto& usage : usages)
      {
        if(!usage.base_binding.null() || usage.global_write)
          continue;

        auto nodei = helper.createAstNodeInfo(usage);
        ret.push_back(makeInfoNode({}, getFileLoc(usage), nodei.astNodeSrcText,
          nodei));
      }

      sortNodeInfoByFilePos(ret, db);

      return ret;
    };

    // Original verson of writes using refs
    auto writes = [this, &astNode_]()
    {
      std::vector<InfoNode> ret;

      auto binding = *(db->load<PythonBinding>(astNode_.id));
      auto usages = helper.queryUsageByAstNode(astNode_);
      for(auto& usage : usages)
      {
        if(usage.base_binding.null() && !usage.global_write)
          continue;

        auto nodei = helper.createAstNodeInfo(usage);
        ret.push_back(makeInfoNode({}, getFileLoc(usage), nodei.astNodeSrcText,
          nodei));
      }

      sortNodeInfoByFilePos(ret, db);

      return ret;
    };

    // possible usage
    auto possible_usages = [this, &astNode_, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      auto binding = *(db->load<PythonBinding>(astNode_.id));
      auto possible_usages = helper.queryPossibleUsages(astNode_);
      sortNodeInfoByPackagePos(ret, possible_usages, { }, infoQuery, helper);

      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);
    switch (queryId)
    {
      case SubQuery::Reads:
        if(astNode_.ast_type == PythonAstNode::AstType::Attribute)
          return getVarRefs(VarRefType::Read);
        else
          return reads();
      case SubQuery::Writes:
        if(astNode_.ast_type == PythonAstNode::AstType::Attribute)
          return getVarRefs(VarRefType::WriteAndDef);
        else
          return writes();
      case SubQuery::PossibleUsage:
        return possible_usages();
      default:
        break;
    }

    return {};
  }
}; // VariableHandler



// ClassHandler
//

class ClassHandler : public TreeHandler
{
  enum class SubQuery
  {
    InheritsFrom = 1,
    InheritsBy,
    Constructors,
    Attributes,
    Methods,
    Usage
  };

public:
  ClassHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
    std::vector<InfoNode> ret;

    auto binding = *(db->load<model::PythonBinding>(astNode_.id));

    // short name
    ret.push_back(makeInfoNode({}, "Name", astNode_.name,
      helper.createAstNodeInfo(binding)));

    // type
    ret.push_back(makeInfoNode({}, "Type", binding.type));

    // declared
    ret.push_back(makeInfoNode({}, "Defined", getFileLoc(binding),
      helper.createAstNodeInfo(binding)));

    // defered queries
    ret.push_back(makeInfoQueryNode({"Inherits from"}, (int)SubQuery::InheritsFrom));
    ret.push_back(makeInfoQueryNode({"Inherited by"}, (int)SubQuery::InheritsBy));
    ret.push_back(makeInfoQueryNode({"Constructors"}, (int)SubQuery::Constructors));
    ret.push_back(makeInfoQueryNode({"Attributes"}, (int)SubQuery::Attributes));
    ret.push_back(makeInfoQueryNode({"Methods"}, (int)SubQuery::Methods));
    ret.push_back(makeInfoQueryNode({"Usage"}, (int)SubQuery::Usage));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(
    const model::PythonAstNode& astNode_,
    const InfoQuery& infoQuery) const override
  {
    using namespace model;

    auto binding = *(db->load<PythonBinding>(astNode_.id));

    
    auto inherits_from = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto bases = db->query<PythonInheritance>(
        queryInheritance::target == binding.id);
      
      for(auto& base : bases)
      {
        auto base_node = *(db->load<PythonAstNode>(base.base));
        ret.push_back(makeInfoNode({}, getFileLoc(base_node), base_node.name,
          helper.createAstNodeInfo(base_node)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto inherits_by = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto targets = db->query<PythonInheritance>(
        queryInheritance::base == astNode_.id
        &&
        queryInheritance::kind != PythonInheritance::Missing);
      
      for(auto& target : targets)
      {
        auto target_node = *(db->load<PythonAstNode>(target.target));
        ret.push_back(makeInfoNode({}, getFileLoc(target_node), target_node.name,
          helper.createAstNodeInfo(target_node)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto constructors = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto classes = db->query<PythonClassDef>(
        queryClassDef::target == binding.id);
      
      for(auto& cls : classes)
      {
        if(cls.constructor.null())
          continue;

        auto cls_node = *(db->load<PythonAstNode>(*(cls.constructor)));
        auto nodei = helper.createAstNodeInfo(cls_node);
        ret.push_back(makeInfoNode({}, getFileLoc(cls_node),
          nodei.astNodeSrcText, nodei));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto attributes = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto attributes = db->query<PythonAttribute>(
        queryAttribute::target == binding.id);
      
      for(auto& attribute : attributes)
      {
        auto attribute_node = *(db->load<PythonAstNode>(attribute.attribute));
        ret.push_back(makeInfoNode({}, getFileLoc(attribute_node),
          attribute_node.name, helper.createAstNodeInfo(attribute_node)));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto methods = [this, &astNode_, &binding]()
    {
      std::vector<InfoNode> ret;

      auto methods = db->query<PythonAstNode>(
        queryAstNode::container_binding == binding.id &&
        queryAstNode::ast_type == PythonAstNode::AstType::Function &&
        queryAstNode::name != "__init__");
      
      for(auto& method : methods)
      {
        auto nodei = helper.createAstNodeInfo(method);
        ret.push_back(makeInfoNode({}, getFileLoc(method),
          nodei.astNodeSrcText, nodei));
      }

      std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

      return ret;
    };

    auto usage = [this, &astNode_, &binding, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      if(infoQuery.filters.empty())
      {
        auto usages = helper.queryUsageByAstNode(astNode_);      
        for(auto& usage : usages)
        {
          if(usage.id == binding.id)
            continue;

          auto nodei = helper.createAstNodeInfo(usage);
          ret.push_back(makeInfoNode({}, getFileLoc(usage), nodei.astNodeSrcText,
            nodei));
        }

        sortNodeInfoByFilePos(ret, db);
      }

      // possible usage:
      auto possible_usages = helper.queryPossibleUsages(astNode_);
      sortNodeInfoByPackagePos(
        ret, possible_usages, { "Possible usage" }, infoQuery, helper);

      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);
    switch (queryId)
    {
      case SubQuery::InheritsFrom:
        return inherits_from();
      case SubQuery::InheritsBy:
        return inherits_by();
      case SubQuery::Constructors:
        return constructors();
      case SubQuery::Attributes:
        return attributes();
      case SubQuery::Methods:
        return methods();
      case SubQuery::Usage:
        return usage();
      default:
        break;
    }

    return {};
  }
}; // ClassHandler



// ModuleHandler
//

class ModuleHandler : public TreeHandler
{
  enum class SubQuery
  {
    Classes = 1,
    Functions,
    Globals
  };

public:
  ModuleHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
    std::vector<InfoNode> ret;

    model::File file;
    db->load<model::File>(astNode_.location.file.object_id(), file);

    // filename
    ret.push_back(makeInfoNode({}, "Name", file.filename,
      helper.createAstNodeInfo(astNode_)));

    // path
    ret.push_back(makeInfoNode({}, "Path", file.path));

    // content
    ret.push_back(makeInfoNode({}, "Content", "Python source"));

    ret.push_back(makeInfoQueryNode({"Classes"}, (int)SubQuery::Classes));
    ret.push_back(makeInfoQueryNode({"Functions"}, (int)SubQuery::Functions));
    ret.push_back(makeInfoQueryNode({"Global variables"}, (int)SubQuery::Globals));

    return ret;
  }

std::vector<InfoNode> getSubInfoTree(
  const model::PythonAstNode& astNode_,
  const InfoQuery& infoQuery) const override
{
  using namespace model;

  auto fid = astNode_.location.file.object_id();

  auto classes = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto classes = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Class &&
      queryAstNode::base_binding.is_not_null() &&
      queryAstNode::location.file == fid);

    for(auto& class_node : classes)
    {
      ret.push_back(makeInfoNode({}, getFileLoc(class_node), class_node.name,
        helper.createAstNodeInfo(class_node)));
    }
    
    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };
    
  auto functions = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto fileResult = db->query<PythonAstNode>(
      queryAstNode::location.file == fid &&
      queryAstNode::ast_type == PythonAstNode::AstType::Module);

    if(fileResult.empty())
      throw std::runtime_error(
        "Module node is not found for file id: " + std::to_string(fid));
    auto module_node = *(fileResult.begin()); // should contains only one element

    auto functions = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Function &&
      queryAstNode::base_binding.is_not_null() &&
      // is a global function
      queryAstNode::container_binding == module_node.id &&
      queryAstNode::location.file == fid);

    for(auto& function : functions)
    {
      ret.push_back(makeInfoNode({}, getFileLoc(function), function.name,
        helper.createAstNodeInfo(function)));
    }

    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };

  auto globals = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto globals = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Variable &&
      queryAstNode::base_binding.is_not_null() &&
      queryAstNode::location.file == fid);

    for(auto& global : globals)
    {
      auto variable_node = db->load<PythonVariable>(global.id);
      if(!variable_node->is_global)
        continue;

      ret.push_back(makeInfoNode({}, getFileLoc(global), global.name,
        helper.createAstNodeInfo(global)));
    }

    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };

  auto queryId = static_cast<SubQuery>(infoQuery.queryId);
  switch (queryId)
  {
    case SubQuery::Classes:
      return classes();
    case SubQuery::Functions:
      return functions();
    case SubQuery::Globals:
      return globals();
    default:
      break;
  }

  return {};
  }
}; // ModuleHandler


// UnknownHandler
//

class UnknownHandler : public TreeHandler
{
  enum class SubQuery
  {
    PossibleDef = 1,
    PossibleUsage
  };

public:
  UnknownHandler(std::shared_ptr<odb::database> db)
    : TreeHandler(db)
  {
  }

  std::vector<InfoNode> getInfoTree(
    const model::PythonAstNode& astNode_) const override
  {
    std::vector<InfoNode> ret;

    ret.push_back(makeInfoNode({}, "Name", astNode_.name,
      helper.createAstNodeInfo(astNode_)));
    ret.push_back(makeInfoNode({}, "Status", "Unresolved"));

    // defered queries
    ret.push_back(makeInfoQueryNode({"Possible definition"}, (int)SubQuery::PossibleDef));
    ret.push_back(makeInfoQueryNode({"Possible usage"}, (int)SubQuery::PossibleUsage));

    return ret;
  }

  std::vector<InfoNode> getSubInfoTree(
    const model::PythonAstNode& astNode_,
    const InfoQuery& infoQuery) const override
  {
    using namespace model;

    auto possible_def = [this, &astNode_, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      // possible resolved definitions:
      auto possible_resolved_defs = helper.queryPossibleDefs(astNode_);
      sortNodeInfoByPackagePos(ret, possible_resolved_defs,
        { }, infoQuery, helper);

      return ret;
    };

    auto possible_usage = [this, &astNode_, &infoQuery]()
    {
      std::vector<InfoNode> ret;

      // unresolved usages:
      auto possible_usages = helper.queryPossibleUsages(astNode_);
      sortNodeInfoByPackagePos(ret, possible_usages, { }, infoQuery, helper);
      
      return ret;
    };

    auto queryId = static_cast<SubQuery>(infoQuery.queryId);
    switch (queryId)
    {
      case SubQuery::PossibleDef:
        return possible_def();
      case SubQuery::PossibleUsage:
        return possible_usage();
      default:
        break;
    }

    return {};
  }
}; // UnknownHandler


std::unique_ptr<TreeHandler> TreeHandler::getHandler(
  std::shared_ptr<odb::database> db, model::PythonAstNode::AstType astType_)
{
  switch(astType_)
  {
    case model::PythonAstNode::AstType::Variable:
    case model::PythonAstNode::AstType::Parameter:
    case model::PythonAstNode::AstType::Attribute:
      return std::unique_ptr<TreeHandler> { new VariableHandler(db) };

    case model::PythonAstNode::AstType::Function:
      return std::unique_ptr<TreeHandler> { new FunctionHandler(db) };

    case model::PythonAstNode::AstType::Class:
      return std::unique_ptr<TreeHandler> { new ClassHandler(db) };

    case model::PythonAstNode::AstType::Module:
    case model::PythonAstNode::AstType::ModuleRef:
      return std::unique_ptr<TreeHandler> { new ModuleHandler(db) };

    case model::PythonAstNode::AstType::Unknown:
    case model::PythonAstNode::AstType::Bindingless:
      return std::unique_ptr<TreeHandler> { new UnknownHandler(db) };

    default:
      return std::unique_ptr<TreeHandler> { new DefaultHandler(db) };
  }
}



//
// FileTreeHandler

std::vector<InfoNode> FileTreeHandler::getInfoTreeForFile(
  const model::FileId& fid)
{
  std::vector<InfoNode> ret;

  model::File file;
  db->load<model::File>(fid, file);

  // filename
  ret.push_back(makeInfoNode({}, "Name", file.filename));

  // path
  ret.push_back(makeInfoNode({}, "Path", file.path));

  // content
  ret.push_back(makeInfoNode({}, "Content", "Python source"));

  ret.push_back(makeInfoQueryNode({"Classes"}, (int)SubQuery::Classes));
  ret.push_back(makeInfoQueryNode({"Functions"}, (int)SubQuery::Functions));
  ret.push_back(makeInfoQueryNode({"Global variables"}, (int)SubQuery::Globals));

  return ret;
}

std::vector<InfoNode> FileTreeHandler::getSubInfoTreeForFile(
  const model::FileId& fid,
  const InfoQuery& infoQuery)
{
  using namespace model;

  auto classes = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto classes = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Class &&
      queryAstNode::base_binding.is_not_null() &&
      queryAstNode::location.file == fid);

    for(auto& class_node : classes)
    {
      ret.push_back(makeInfoNode({}, getFileLoc(class_node), class_node.name,
        helper.createAstNodeInfo(class_node)));
    }
    
    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };
    
  auto functions = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto fileResult = db->query<PythonAstNode>(
      queryAstNode::location.file == fid &&
      queryAstNode::ast_type == PythonAstNode::AstType::Module);

    if(fileResult.empty())
      throw std::runtime_error(
        "Module node is not found for file id: " + std::to_string(fid));
    auto module_node = *(fileResult.begin()); // should contains only one element

    auto functions = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Function &&
      queryAstNode::base_binding.is_not_null() &&
      // is a global function
      queryAstNode::container_binding == module_node.id &&
      queryAstNode::location.file == fid);

    for(auto& function : functions)
    {
      ret.push_back(makeInfoNode({}, getFileLoc(function), function.name,
        helper.createAstNodeInfo(function)));
    }

    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };

  auto globals = [this, &fid]()
  {
    std::vector<InfoNode> ret;

    auto globals = db->query<PythonAstNode>(
      queryAstNode::ast_type == PythonAstNode::AstType::Variable &&
      queryAstNode::base_binding.is_not_null() &&
      queryAstNode::location.file == fid);

    for(auto& global : globals)
    {
      auto variable_node = db->load<PythonVariable>(global.id);
      if(!variable_node->is_global)
        continue;

      ret.push_back(makeInfoNode({}, getFileLoc(global), global.name,
        helper.createAstNodeInfo(global)));
    }

    std::sort(ret.begin(), ret.end(), compInfoNodeByPos());

    return ret;
  };

  auto queryId = static_cast<SubQuery>(infoQuery.queryId);
  switch (queryId)
  {
    case SubQuery::Classes:
      return classes();
    case SubQuery::Functions:
   
   return functions();
    case SubQuery::Globals:
      return globals();
    default:
      break;
  }

  return {};
}



//
// Helper methods of TreeHandler



InfoNode makeInfoNode(
  const std::vector<std::string>& category_,
  const std::string& label_,
  const std::string& value_,
  const AstNodeInfo& astValue_)
{
  InfoNode infoNode;
  infoNode.category = std::move(category_);
  infoNode.label = label_;
  infoNode.value = value_;
  infoNode.astValue = astValue_;
  return infoNode;
}

InfoNode makeInfoQueryNode(
  const std::vector<std::string>& category,
  const int queryId,
  const std::vector<std::string>& filters)
{
  InfoQuery query;
  query.queryId = queryId;
  query.filters = std::move(filters);

  InfoNode ret;
  ret.category = std::move(category);
  ret.query = query;

  return ret;
}

void sortNodeInfoByFilePos(
  std::vector<InfoNode>& vec,
  std::shared_ptr<odb::database> db_)
{
  using namespace model;

  std::map<std::string, std::set<InfoNode, compInfoNodeByPos> > tmp;
  for(auto& inode : vec)
  {
    const auto& fid = inode.astValue.range.file.fid;

    if(tmp.count(fid) == 0)
      tmp.insert({ fid, {inode} });
    else
      tmp[fid].insert(inode);
  }

  std::vector<InfoNode> ret;

  for(auto& entity : tmp)
  {
    const auto& fid = entity.first;
    const auto filename = db_->load<File>(std::stoull(fid))->filename;

    for(auto& inode : entity.second)
    {
      InfoNode ret_inode = inode;
      ret_inode.category.push_back(
        filename  + " (" + std::to_string(entity.second.size()) + ")");

      ret.push_back(ret_inode);
    }
  }

  vec = ret;
}

template<typename AstNodeIterableCollection>
std::vector<InfoNode> sortNodeInfoByPackagePos(
  std::vector<InfoNode>& ret,
  AstNodeIterableCollection& collection,
  const std::vector<std::string> category_,
  const InfoQuery& infoQuery,
  const PythonQueryHelper& helper)
{
  auto split_by = boost::regex("\\.");

  if(infoQuery.filters.empty())
  {
    std::map<std::string, int> packages;

    for(auto& possible : collection)
    {
      auto path = possible.abv_qname;

      if(packages.count(path) == 0)
        packages.insert({ path, 1 });
      else
        packages[path] += 1;
    }

    for(auto& pkq_entity : packages)
    {
      std::vector<std::string> pkgs;
      boost::split_regex(pkgs, pkq_entity.first, split_by);

      std::vector<std::string> pkgs_ret = pkgs;
      pkgs_ret.back() += " (" + std::to_string(pkq_entity.second) + ")";

      auto category = category_;
      category.insert(category.end(), pkgs_ret.begin(), pkgs_ret.end());
      ret.push_back(makeInfoQueryNode(category, infoQuery.queryId, pkgs));
    }
  }
  else
  {
    for(auto& possible : collection)
    {
      std::vector<std::string> pkg;
      boost::split_regex(pkg, possible.abv_qname, split_by);

      if(pkg != infoQuery.filters)
        continue;

      auto nodei = helper.createAstNodeInfo(possible);
      ret.push_back(
        makeInfoNode({}, getFileLoc(possible),
          nodei.astNodeSrcText, nodei));
    }
  }

  std::sort(ret.begin(), ret.end(), compInfoNodeByPos());
  return ret;
}

} // python
} // language
} // service
} // cc
