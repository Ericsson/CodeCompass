#include <pythonservice/pythonqueryhelper.h>

#include <stdexcept>
#include <algorithm>
#include <set>

#include <util/streamlog.h>
#include <util/graph.h>

#include <pythonservice/pythonpossibles.h>

namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{

const std::set<std::string> PythonQueryHelper::common_python_keywords = 
{
  "and", "as", "assert", "break", "class", "continue",
  "def", "del", "elif", "else", "except", "exec", "finally", "for", "from", 
  "if", "import", "in", "is", "lambda", "not", "or", "pass", "print", "raise", 
  "try", "while", "with", "yield", "None", "True", "False", "len", "append",
  "isinstance", "sys", "str", "int", "super", "object", "ValueError", "cfg",
  "join", "dict", "getattr", "Exception", "list", "hasattr", "TypeError",
  "property", "get", "type", "time", "datetime", "KeyError", "set", "tuple",
  "NotImplementedError", "ImportError", "AttributeError", "open", "range",
  "classmethod", "repr", "unicode", "split", "setattr", "float", "extend",
  "close", "write", "pop", "update", "bool", "signal", "exists", "basestring",
  "replace", "IOError", "global", "return"
};

model::PythonAstNode PythonQueryHelper::queryAstNodeByPosition(
  const ::cc::service::core::FilePosition & fpos_) const
{
  auto nodesResult = db->query<model::PythonAstNode>(
    // in file
    queryAstNode::location.file == std::stoull(fpos_.file.fid)
    &&
    // in range
    predContainsPosition<queryAstNode>(
      queryAstNode::location.range,
      fpos_.pos)
    &&
    // not module
    queryAstNode::ast_type != model::PythonAstNode::AstType::Module);

  if(nodesResult.empty())
    throw core::InvalidPosition();

  return min_element<model::PythonAstNode>(nodesResult.begin(),
    nodesResult.end(),
    [](const model::PythonAstNode & lhs, const model::PythonAstNode & rhs)
    {
      return lhs.location.range < rhs.location.range;
    });
}

std::vector<model::PythonBinding> PythonQueryHelper::queryBindingByAstNode(
  const model::PythonAstNode & astNode_) const
{
  return queryBindingByAstNode(astNode_.id);
}

std::vector<model::PythonBinding> PythonQueryHelper::queryBindingByAstNode(
  const model::PythonAstNode::pktype & astNodeId_) const
{
  std::set<model::PythonBinding> tmp;

 {
  auto bindings = db->query<model::PythonAllBindingRefToBind>(
    queryReference::node == astNodeId_);

  std::for_each(bindings.begin(), bindings.end(),
    [&tmp, this](const model::PythonAllBindingRefToBind& binding)
    {
      model::PythonBinding bind = { binding.id, binding.name,
        binding.formatted_qname, binding.mangled_name, binding.kind,
        binding.type, createFileLoc(binding), binding.documentation };
      
      tmp.insert(bind);
    });
  }

  {
  auto bindings = db->query<model::PythonAllBindingBindToBind>(
    queryBinding::id == astNodeId_);

  std::for_each(bindings.begin(), bindings.end(),
    [&tmp, this](const model::PythonAllBindingBindToBind& binding)
    {
      model::PythonBinding bind = { binding.id, binding.name,
        binding.formatted_qname, binding.mangled_name, binding.kind,
        binding.type, createFileLoc(binding), binding.documentation };
      
      tmp.insert(bind);
    });
  }

  return std::vector<model::PythonBinding>(tmp.begin(), tmp.end());
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryUsageByAstNode(
  const model::PythonAstNode & astNode_) const
{
  return queryUsageByAstNode(astNode_.id);
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryUsageByAstNode(
  const model::PythonAstNode::pktype & astNodeId_) const
{
  std::set<model::PythonAstNode> tmp;

  // ref -> ref
  insertRefs<model::PythonRefNodeRefToRef>(
    queryReference::node, astNodeId_, tmp);

  // ref -> bind
  insertRefs<model::PythonRefNodeRefToBind>(
    queryReference::node, astNodeId_, tmp);

  // bind -> bind
  insertRefs<model::PythonRefNodeBindToBind>(
    queryBinding::id, astNodeId_, tmp);

  // bind -> ref
  insertRefs<model::PythonRefNodeBindToRef>(
    queryBinding::id, astNodeId_, tmp);

  return std::vector<model::PythonAstNode>(tmp.begin(), tmp.end());
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryPossibleDefs(
  const model::PythonAstNode::pktype & astNodeId_,
  const std::vector<model::PythonBinding> & resolvedDefs_,
  const bool isLimited) const
{
  return queryPossibleDefs(*(db->load<model::PythonAstNode>(astNodeId_)),
    resolvedDefs_, isLimited);
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryPossibleDefs(
  const model::PythonAstNode & astNode_,
  const std::vector<model::PythonBinding> & resolvedDefs_,
  const bool isLimited) const
{
  return PythonPossibleHandler::getPossibleDefs(astNode_, db, isLimited); 
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryPossibleUsages(
  const model::PythonAstNode::pktype & astNodeId_,
  const std::vector<model::PythonBinding> & resolvedUsages_) const
{
  return queryPossibleUsages(*(db->load<model::PythonAstNode>(astNodeId_)),
    resolvedUsages_);
}

std::vector<model::PythonAstNode> PythonQueryHelper::queryPossibleUsages(
  const model::PythonAstNode & astNode_,
  const std::vector<model::PythonBinding> & resolvedUsages_) const
{
  return PythonPossibleHandler::getPossibleUsages(astNode_, db);
}

/*template<typename QueryType>
odb::result<QueryType> PythonQueryHelper::queryWithLimit(
  const odb::query<QueryType>& query,
  const int limit,
  const int offset) const
{
  return odb::result<QueryType>(
    query + " LIMIT "  + odb::query<QueryType>::_val(limit)
          + " OFFSET " + odb::query<QueryType>::_val(offset));
}*/

const std::string PythonQueryHelper::getFormattedQName(
  const model::PythonBinding& binding) const
{
  std::string ret = (!binding.formatted_qname.null())
    ? *(binding.formatted_qname)
    : binding.location.file.load()->filename;

  return ret + ':' + std::to_string(binding.location.range.start.line);
}

const std::string PythonQueryHelper::getVarRefMangledName(
  const model::PythonAstNode::pktype& astNodeId_) const
{
  // Get mangled name by AstNode
  auto mangledNameRef = db->query<model::PythonVariableRef>(
    queryVariableRef::astNode == astNodeId_);
  if (mangledNameRef.empty())
  {
    SLog(util::DEBUG) << "Mangled name for " << astNodeId_ << " not found!";
    return "";
  }

  return mangledNameRef.begin()->mangledName;
}

odb::result<model::PythonVariableRef> PythonQueryHelper::getVarRefsForAstNode(
  const model::PythonAstNode::pktype& astNodeId_,
  const queryVariableRef extraCond_) const
{
  std::string mangledName = getVarRefMangledName(astNodeId_);
  if (mangledName.empty())
  {
    return odb::result<model::PythonVariableRef>();
  }

  queryVariableRef query = queryVariableRef::mangledName == mangledName;
  if (!extraCond_.empty())
  {
    query = query && extraCond_;
  }

  return db->query<model::PythonVariableRef>(query);
}

std::string PythonQueryHelper::getFunctionCallDiagram(
    const model::PythonAstNode& astNode_) const
{
  util::Graph graph("Python Function call diagram");

  auto clikkedNode = graph.addNode();
  graph.setAttribute(clikkedNode, "label", astNode_.name);
  graph.setAttribute(clikkedNode, "id", std::to_string(astNode_.id));
  graph.setAttribute(clikkedNode, "style", "filled");
  graph.setAttribute(clikkedNode, "fildcolor", "gold");

  auto calls = db->query<model::PythonAstNode>(
    queryAstNode::container_binding == astNode_.id &&
    (queryAstNode::ast_type == model::PythonAstNode::AstType::Call ||
    queryAstNode::ast_type == model::PythonAstNode::AstType::Function) &&
    queryAstNode::base_binding.is_null());

  std::set<std::string> drawen_names;
  for(auto& call : calls)
  {
    if(drawen_names.count(call.name) != 0)
      continue;

    auto call_node = graph.addNode();
    graph.setAttribute(call_node, "label", call.name);

    auto bindings = queryBindingByAstNode(call.id);
    graph.setAttribute(call_node, "id", std::to_string(bindings[0].id)); //TODO: support multiple binding

    auto edge = graph.addEdge(clikkedNode, call_node);
    graph.setAttribute(edge, "color", "blue");

    drawen_names.insert(call.name);
  }

  auto callers = queryUsageByAstNode(astNode_);
        
  drawen_names.clear();
  for(auto& caller : callers)
  {
    if(caller.id == astNode_.id || 
      db->find<model::PythonFunctionDef>(caller.id) || caller.container_binding.null())
      continue;

    auto container_binding = *db->load<model::PythonAstNode>(
      *caller.container_binding);

    if(drawen_names.count(container_binding.name) != 0)
      continue;

    auto caller_node = graph.addNode();
    graph.setAttribute(caller_node, "label", container_binding.name);

    graph.setAttribute(caller_node, "id", std::to_string(container_binding.id));

    auto edge = graph.addEdge(caller_node, clikkedNode);
    graph.setAttribute(edge, "color", "red");

    drawen_names.insert(container_binding.name);
  }

  return graph.output(util::Graph::Format::SVG);
}

const AstNodeInfo PythonQueryHelper::createAstNodeInfoByAstNodeId(
  const model::PythonAstNode::pktype & astNodeId_) const
{
  return createAstNodeInfo<model::PythonAstNode>(
    (*(db->load<model::PythonAstNode>(astNodeId_))));
}

const AstNodeInfo PythonQueryHelper::createAstNodeInfoByBindingId(
  const model::PythonBinding::pktype & astNodeId_) const
{
  return createAstNodeInfo<model::PythonBinding>(
    (*(db->load<model::PythonBinding>(astNodeId_))));
}

template<typename EntityType>
const AstNodeInfo PythonQueryHelper::createAstNodeInfo(
  const EntityType & entity) const
{
  AstNodeInfo ret;

  ret.astNodeId.astNodeId = std::to_string(entity.id);

  if(entity.location.file)
  {
    ret.range.file.fid = std::to_string(entity.location.file.object_id());
   
    if(entity.location.file.load()->content)
    {
      const auto& content = entity.location.file->content.load()->content;

      model::Range range(
        model::Position(entity.location.range.start.line, 1),
        model::Position(entity.location.range.end.line, std::string::npos));

      const auto ROW_SIZE = 100;

      ret.astNodeSrcText = textRange(content, range, ROW_SIZE);
    }
  }

  ret.range.range.startpos.line = entity.location.range.start.line;
  ret.range.range.startpos.column = entity.location.range.start.column;
 
  ret.range.range.endpos.line = entity.location.range.end.line;
  ret.range.range.endpos.column = entity.location.range.end.column;

  return ret;
}

core::RangedHitCountResult PythonQueryHelper::makeRangedHitCountResult(
  const std::set<model::FileId>& resultSet,
  const int pageSize,
  const int pageNo) const
{
  core::RangedHitCountResult result;

  int totalCnt = 0;

  for(auto& fid : resultSet)
  {
    if (totalCnt >= (pageNo * pageSize)
      && totalCnt < ((pageNo + 1) * pageSize))
    {
      auto file = db->load<model::File>(fid);

      core::HitCountResult hitcount;
      hitcount.finfo.file.fid = std::to_string(file->id);
      hitcount.finfo.name = file->filename;
      hitcount.finfo.path = file->path;

      result.results.push_back(hitcount);
    }
    ++totalCnt;
  }

  result.firstFileIndex = (pageNo * pageSize) + 1;
  result.totalFiles = totalCnt;

  return result;
}

core::RangedHitCountResult PythonQueryHelper::getReferencesPage(
  const core::AstNodeId& astNodeId,
  const int pageSize,
  const int pageNo) const
{
  std::set<model::FileId> fids;
  for(auto& usage : queryUsageByAstNode(
    std::stoull(astNodeId.astNodeId)))
  {
    fids.insert(usage.location.file.object_id());
  }

  for(auto& usage : queryPossibleUsages(
    std::stoull(astNodeId.astNodeId)))
  {
    fids.insert(usage.location.file.object_id());
  }

  return makeRangedHitCountResult(fids, pageSize, pageNo);
}


} // python
} // language
} // service
} // cc
