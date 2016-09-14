#include <pythonservice/pythonpossibles.h>

#include <util/streamlog.h>

namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{

class PythonVariablePossHandler : public PythonPossibleHandler
{
public:  
  PythonVariablePossHandler(std::shared_ptr<odb::database> db_)
    : PythonPossibleHandler(db_)
  {}

  std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_) const override
  {
    using namespace model;

    auto result = db->query<PythonAstNode>(
      queryAstNode::name == astNode_.name
      &&
      (
        queryAstNode::ast_type == PythonAstNode::AstType::Variable
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Attribute
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Parameter
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Unknown
        ||
        queryAstNode::ast_type == PythonAstNode::AstType::Bindingless
      )
      &&
      queryAstNode::id != astNode_.id);

    std::vector<PythonAstNode> ret;
    for(auto& node : result)
      ret.push_back(node);

    return ret;
  }
};

class PythonFunctionPossHandler : public PythonPossibleHandler
{
public:
  PythonFunctionPossHandler(std::shared_ptr<odb::database> db_)
    : PythonPossibleHandler(db_)
  {}

  std::vector<model::PythonAstNode> getPossibleDefs(
    const model::PythonAstNode& astNode_,
    bool isLimited = false) const override
  {
    using namespace model;
    std::vector<PythonAstNode> ret;
    odb::query<PythonFunctionDefWithName> query;
    
    if(astNode_.ast_type == PythonAstNode::AstType::Call
       ||
       astNode_.ast_type == PythonAstNode::AstType::Bindingless
       ||
       astNode_.ast_type == PythonAstNode::AstType::Unknown){
      auto call = *(db->load<PythonFunctionCall>(astNode_.id));
      query = (
        queryBinding::name == astNode_.name
        &&
        queryFunctionDef::min_param_num <= call.arg_num
        &&
        queryFunctionDef::max_param_num >= call.arg_num);
    }
    else
    {
      // if it is a function pointer, get it's binding
      if(astNode_.base_binding.null()){
        auto b = helper.queryBindingByAstNode(astNode_.id);
        
        auto fd = *(db->load<PythonFunctionDef>(b.begin()->id));
        query = (
        queryBinding::name == astNode_.name
        &&
        queryFunctionDef::min_param_num == fd.min_param_num
        &&
        queryFunctionDef::max_param_num == fd.max_param_num);
      } else {
          auto fd = *(db->load<PythonFunctionDef>(astNode_.id));
          query = (
            queryBinding::name == astNode_.name
            &&
            queryFunctionDef::min_param_num == fd.min_param_num
            &&
            queryFunctionDef::max_param_num == fd.max_param_num);
      }
    }

    auto result = (isLimited) 
      ? helper.queryWithLimit(query, 11)
      : db->query<PythonFunctionDefWithName> (query);

    for(auto& pfunc_def : result)
    {
      auto def = *(db->load<PythonAstNode>(pfunc_def.id));
      ret.push_back(def);
    }

    return ret;
  }

  std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_) const override
  {
    using namespace model;
    std::vector<PythonAstNode> ret;

    auto defs = helper.queryBindingByAstNode(astNode_);

    for(auto& def : defs)
    {
      auto fdef_node = *(db->load<PythonFunctionDef>(def.id));
      auto pcalled_by = db->query<PythonFunctionCallWithName>(
        queryAstNode::name == astNode_.name
        &&
        queryFunctionCall::arg_num >= fdef_node.min_param_num
        &&
        queryFunctionCall::arg_num <= fdef_node.max_param_num);

      for(auto& pcall : pcalled_by)
      {
        auto node = *(db->load<PythonAstNode>(pcall.id));
        ret.push_back(node);
      }
    }

    return ret;
  }
};

class PythonClassPossHandler : public PythonPossibleHandler
{
public:
  PythonClassPossHandler(std::shared_ptr<odb::database> db_)
    : PythonPossibleHandler(db_)
  {}

  std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_) const override
  {
    using namespace model;

    auto result = db->query<PythonAstNode>(
      queryAstNode::name == astNode_.name
      &&
      queryAstNode::base_binding.is_null()
      &&
      queryAstNode::ast_type == PythonAstNode::AstType::Class);
    
    std::vector<PythonAstNode> ret;
    for(auto& node : result)
      ret.push_back(node);

    return ret;
  }
};

class PythonUnknownPossHandler : public PythonPossibleHandler
{
public:
  PythonUnknownPossHandler(std::shared_ptr<odb::database> db_)
    : PythonPossibleHandler(db_)
  {}

  std::vector<model::PythonAstNode> getPossibleDefs(
    const model::PythonAstNode& astNode_,
    const bool isLimited = false) const override
  {
    using namespace model;
    if(astNode_.ast_type == PythonAstNode::AstType::Unknown)
    {
      auto unknown_node = *(db->load<PythonUnknown>(astNode_.id));
      if(unknown_node.kind == PythonUnknown::Kind::Attribute)
      {
        odb::query<PythonVariableRef> query (
          queryVariableRef::mangledName.like("%." + astNode_.name) &&
          queryVariableRef::refType == PythonVariableRef::RefType::Definition);

        auto result = (isLimited) 
          ? helper.queryWithLimit(query, 11)
          : db->query<PythonVariableRef> (query);

        std::vector<PythonAstNode> ret;
        for(auto& vref :result)
          ret.push_back(*(vref.astNode.load()));

        return ret;
      }
    }
    odb::query<PythonAstNode> query (
      queryAstNode::name == astNode_.name &&
      queryAstNode::base_binding.is_not_null());

    auto result = (isLimited) 
      ? helper.queryWithLimit(query, 11)
      : db->query<PythonAstNode> (query);

    return { result.begin(), result.end() };
  }

  std::vector<model::PythonAstNode> getPossibleUsages(
    const model::PythonAstNode& astNode_) const override
  {
    using namespace model;

    if(astNode_.ast_type == PythonAstNode::AstType::Unknown)
    {
      auto unknown_node = *(db->load<PythonUnknown>(astNode_.id));
      if(unknown_node.kind == PythonUnknown::Kind::Attribute)
      {
        auto pdefs = db->query<PythonVariableRef>(
          queryVariableRef::mangledName.like("%" + astNode_.name) &&
          (
            queryVariableRef::refType == PythonVariableRef::RefType::Read ||
            queryVariableRef::refType == PythonVariableRef::RefType::Write
          ));

        std::vector<PythonAstNode> ret;
        for(auto& vref : pdefs)
          ret.push_back(*(vref.astNode.load()));

        ret.push_back(astNode_);
        return ret;
      }
    }

    auto result = db->query<PythonAstNode>(
      queryAstNode::name == astNode_.name &&
      queryAstNode::base_binding.is_null());
    
    std::vector<PythonAstNode> ret;
    for(auto& node : result)
      ret.push_back(node);

    return ret;
  }
};

class PythonDefaultPossHandler : public PythonPossibleHandler
{
public:
  PythonDefaultPossHandler(std::shared_ptr<odb::database> db_)
    : PythonPossibleHandler(db_)
  {}

  std::vector<model::PythonAstNode> getPossibleDefs(
    const model::PythonAstNode& astNode_,
    const bool isLimited = false) const override
  {
    using namespace model;

    odb::query<PythonAstNode> query(
      queryAstNode::name == astNode_.name &&
      queryAstNode::base_binding.is_not_null());

    auto result = (isLimited) 
      ? helper.queryWithLimit(query, 11)
      : db->query<PythonAstNode> (query);

    return { result.begin(), result.end() };
  }
};

std::vector<model::PythonAstNode> PythonPossibleHandler::getPossibleDefs(
    const model::PythonAstNode& astNode_,
    std::shared_ptr<odb::database> db_,
    const bool isLimited)
  {
    std::unique_ptr<PythonPossibleHandler> handler;
    switch(astNode_.ast_type)
    { 
      case model::PythonAstNode::AstType::Function:
      case model::PythonAstNode::AstType::Call:
        handler.reset(new PythonFunctionPossHandler(db_));
        break;
  
      case model::PythonAstNode::AstType::Bindingless:
      case model::PythonAstNode::AstType::Unknown:
        if(db_->find<model::PythonFunctionCall>(astNode_.id) != nullptr)
          handler.reset(new PythonFunctionPossHandler(db_));
        else
          handler.reset(new PythonUnknownPossHandler(db_));
        break;

      case model::PythonAstNode::AstType::Parameter:
        return { };

      default:
        handler.reset(new PythonDefaultPossHandler(db_));
        break;
    }

    return handler->getPossibleDefs(astNode_, isLimited);
  }

  std::vector<model::PythonAstNode> PythonPossibleHandler::getPossibleUsages(
    const model::PythonAstNode& astNode_,
    std::shared_ptr<odb::database> db_)
  {
    std::unique_ptr<PythonPossibleHandler> handler;

    switch(astNode_.ast_type)
    {
      case model::PythonAstNode::AstType::Variable:
      case model::PythonAstNode::AstType::Attribute:
        handler.reset(new PythonVariablePossHandler(db_));
        break;

      case model::PythonAstNode::AstType::Parameter:
        return { };
      
      case model::PythonAstNode::AstType::Function:
      case model::PythonAstNode::AstType::Call:
        handler.reset(new PythonFunctionPossHandler(db_));
        break;

      case model::PythonAstNode::AstType::Class:
        handler.reset(new PythonClassPossHandler(db_));
        break;
      
      case model::PythonAstNode::AstType::Bindingless:
      case model::PythonAstNode::AstType::Unknown:
        handler.reset(new PythonUnknownPossHandler(db_));
        break;
      
      default:
        handler.reset(new PythonDefaultPossHandler(db_));
        break;
    }

    return handler->getPossibleUsages(astNode_);
  }

} // python
} // language
} // service
} // cc