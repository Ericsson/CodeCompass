#include <service/pythonservice.h>
#include <projectservice/projectservice.h>
#include <util/dbutil.h>
#include <model/pyname.h>
#include "diagram.h"

namespace cc
{
namespace service
{
namespace language
{

PythonServiceHandler::PythonServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _datadir(datadir_), _context(context_) {}

void PythonServiceHandler::getFileTypes(
  std::vector<std::string>& return_) 
{
  return_.push_back("PY");
  return_.push_back("Dir");
  return;
}

void PythonServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId_);

  PythonServiceHandler::setInfoProperties(return_, pyname);
  return;
}

void PythonServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  model::PYName node = PythonServiceHandler::queryNodeByPosition(fpos_);
  PythonServiceHandler::setInfoProperties(return_, node);
  return;
}

void PythonServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId_);

  core::ProjectServiceHandler projectService(_db, _datadir, _context);
  std::string content;
  projectService.getFileContent(content, std::to_string(pyname.file_id));

  if(!content.empty())
  {
    return_ = cc::util::textRange(
      content,
      pyname.line_start,
      pyname.column_start,
      pyname.line_end,
      pyname.column_end);
  }
  return;
}

void PythonServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId_);

  if(!pyname.full_name.empty())
  {
    return_.emplace("Full name", pyname.full_name);
  }

  return_.emplace("Builtin", PythonServiceHandler::boolToString(pyname.is_builtin));

  if(!pyname.type_hint.empty())
  {
    return_.emplace("Type hint", pyname.type_hint);
  }

  return_.emplace("Function call", PythonServiceHandler::boolToString(pyname.is_call));

#ifndef NDEBUG
    return_.emplace("ID", std::to_string(pyname.id));
    return_.emplace("REF_ID", std::to_string(pyname.ref_id));
    return_.emplace("DEFINITION", PythonServiceHandler::boolToString(pyname.is_definition));
#endif

  return;
}

void PythonServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  const model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId_);

  if (pyname.is_import == true) return;

  if (pyname.is_definition == true && pyname.type == "function") {
      return_.emplace("Function call", FUNCTION_CALL);
  }

  if (pyname.is_definition == true && pyname.type == "class") {
      return_.emplace("Class Overview", CLASS_OVERVIEW);
  }

  // Usage diagrams
  const size_t count = PythonServiceHandler::queryReferences(astNodeId_, USAGE).size();

  if (count > 0 && pyname.is_definition == true) {
    if (pyname.type == "function") {
      return_.emplace("Function usage", FUNCTION_USAGE);
    } else if (pyname.type == "class") {
      return_.emplace("Class Usage", CLASS_USAGE);
    }
  }

  return;
}

void PythonServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  python::Diagram diagram(_db, _datadir, _context);

  model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId_);
  util::Graph graph = [&]()
  {
    switch (diagramId_)
    {
      case FUNCTION_CALL:
        return diagram.getFunctionCallDiagram(pyname);
      case FUNCTION_USAGE:
      case CLASS_USAGE:
        return diagram.getUsageDiagram(pyname);
      case CLASS_OVERVIEW:
        return diagram.getClassDiagram(pyname);
      default:
        return util::Graph();
    }
  }();

  if (graph.nodeCount() != 0) {
    if (diagramId_ == CLASS_OVERVIEW) {
      return_ = graph.output(util::Graph::CAIRO_SVG);
    } else {
      return_ = graph.output(util::Graph::SVG);
    }
  }
  
  return;
}

void PythonServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  python::Diagram diagram(_db, _datadir, _context);

  util::Graph graph = [&]()
  {
    switch (diagramId_)
    {
      case FUNCTION_CALL:
        return diagram.getFunctionCallDiagramLegend();
      case FUNCTION_USAGE:
      case CLASS_USAGE:
        return diagram.getUsageDiagramLegend();
      default:
        return util::Graph();
    }
  }();

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);

  return;
}

void PythonServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return_.emplace("Module dependency", MODULE_DEPENDENCY);
  return;
}

void PythonServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  python::Diagram diagram(_db, _datadir, _context);

  util::Graph graph = [&]()
  {
    switch (diagramId_)
    {
      case MODULE_DEPENDENCY:
        return diagram.getModuleDiagram(fileId_);
      default:
        return util::Graph();
    }
  }();

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);

  return;
}

void PythonServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  python::Diagram diagram(_db, _datadir, _context);

  util::Graph graph = [&]()
  {
    switch (diagramId_)
    {
      case MODULE_DEPENDENCY:
        return diagram.getModuleDiagramLegend();
      default:
        return util::Graph();
    }
  }();

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);

  return;
}

void PythonServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return_.emplace("Definition", DEFINITION);
  return_.emplace("Usage", USAGE);
  return_.emplace("Parent", PARENT);
  return_.emplace("Parameters", PARAMETER);
  return_.emplace("Caller", CALLER);

  model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId);

  if(pyname.type == "function" && pyname.is_definition)
  {
    return_.emplace("Method", METHOD);
    return_.emplace("Local variables", LOCAL_VAR);
    return_.emplace("This calls", THIS_CALLS);
  }

  std::vector<model::PYName> nodes = PythonServiceHandler::queryReferences(astNodeId, DEFINITION);

  if(!nodes.empty())
  {
    model::PYName def = *nodes.begin();
    if(def.type == "class")
    {
      return_.emplace("Method", METHOD);
      return_.emplace("Data member", DATA_MEMBER);
      return_.emplace("Base class", BASE_CLASS);
    }
  }

  return;
}

void PythonServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  LOG(info) << "astNodeID: " << astNodeId_;

  std::vector<model::PYName> nodes = PythonServiceHandler::queryReferences(astNodeId_, referenceId_);

  for(const model::PYName& pyname : nodes)
  {
    AstNodeInfo info;
    PythonServiceHandler::setInfoProperties(info, pyname);
    return_.push_back(info);
  }

  return;
}

std::int32_t PythonServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  LOG(info) << "astNodeID: " << astNodeId_;

  return PythonServiceHandler::queryReferences(astNodeId_, referenceId_).size();
}

void PythonServiceHandler::getReferencesInFile(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const core::FileId& fileId_,
  const std::vector<std::string>& tags_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::int32_t pageSize_,
  const std::int32_t pageNo_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileReferences(
  std::vector<AstNodeInfo>& return_,
  const core::FileId& fileId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

std::int32_t PythonServiceHandler::getFileReferenceCount(
  const core::FileId& fileId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return 0;
}

void PythonServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileRange& range_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

model::PYName PythonServiceHandler::queryNodeByID(const std::string& id)
{
  return _transaction([&]()
  {
    model::PYName pyname;
    odb::result<model::PYName> nodes = _db->query<model::PYName>(odb::query<model::PYName>::id == std::stoull(id));

    if(!nodes.empty())
    {
      pyname = *nodes.begin();
    }else{
      LOG(info) << "[PYSERVICE] Node not found! (id = " << id << ")";
      core::InvalidId ex;
      ex.__set_msg("Node not found!");
      throw ex;
    }

    return pyname;
  });
}

model::PYName PythonServiceHandler::queryNodeByPosition(const core::FilePosition& fpos)
{
  return _transaction([&]()
  {
    model::PYName pyname;
    odb::result<model::PYName> nodes = _db->query<model::PYName>(
      odb::query<model::PYName>::file_id == std::stoull(fpos.file) &&
      odb::query<model::PYName>::line_start == fpos.pos.line &&
      odb::query<model::PYName>::column_start <= fpos.pos.column &&
      odb::query<model::PYName>::column_end >= fpos.pos.column
    );

    if(!nodes.empty())
    {
      pyname = *nodes.begin();

      for(const model::PYName& p : nodes)
      {
        if(p.value.size() < pyname.value.size())
        {
          pyname = p;
        }
      }
    }else{
      LOG(info) << "[PYSERVICE] Node not found! (line = " << fpos.pos.line << " column = " << fpos.pos.column << ")";
      core::InvalidInput ex;
      ex.__set_msg("Node not found!");
      throw ex;
    }

    return pyname;
  });
}

std::vector<model::PYName> PythonServiceHandler::queryNodes(const odb::query<model::PYName>& odb_query)
{
  return _transaction([&](){
    const odb::query<model::PYName> order_by = "ORDER BY" + odb::query<model::PYName>::is_builtin + "," + odb::query<model::PYName>::line_start + "," + odb::query<model::PYName>::column_start;
    const odb::query<model::PYName> q = odb_query + order_by;

    odb::result<model::PYName> nodes = _db->query(q);

    return std::vector<model::PYName>(nodes.begin(), nodes.end());
  });
}

std::vector<model::PYName> PythonServiceHandler::queryReferences(const core::AstNodeId& astNodeId, const std::int32_t referenceId)
{
  return _transaction([&](){
    odb::result<model::PYName> nodes;

    const model::PYName pyname = PythonServiceHandler::queryNodeByID(astNodeId);
    const odb::query<model::PYName> order_by = "ORDER BY" + odb::query<model::PYName>::line_start + "," + odb::query<model::PYName>::column_start;

    switch (referenceId)
    {
      case DEFINITION:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::ref_id == pyname.ref_id && odb::query<model::PYName>::is_definition == true && odb::query<model::PYName>::is_import == false) + order_by);
        break;
      case USAGE:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::ref_id == pyname.ref_id && odb::query<model::PYName>::is_definition == false && odb::query<model::PYName>::id != pyname.id) + order_by);
        break;
      case METHOD:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.ref_id && odb::query<model::PYName>::type == "function" && odb::query<model::PYName>::is_definition == true) + order_by);
        break;
      case LOCAL_VAR:
      case DATA_MEMBER:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.ref_id && odb::query<model::PYName>::type == "statement" && odb::query<model::PYName>::is_definition == true) + order_by);
        break;
      case PARENT:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::id == pyname.parent) + order_by);
        break;
      case PARENT_FUNCTION:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::id == pyname.parent_function) + order_by);
        break;
      case PARAMETER:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.ref_id && odb::query<model::PYName>::type == "astparam" && odb::query<model::PYName>::is_definition == true) + order_by);
        break;
      case CALLER:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::ref_id == pyname.ref_id && odb::query<model::PYName>::is_definition == false && odb::query<model::PYName>::is_call == true && odb::query<model::PYName>::id != pyname.id) + order_by);
        break;
      case THIS_CALLS:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.id && odb::query<model::PYName>::is_call == true) + order_by);
        break;
      case ANNOTATION:
        nodes = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.id && odb::query<model::PYName>::type == "annotation") + order_by);
        break;
      case BASE_CLASS:
        odb::result<model::PYName> bases = _db->query<model::PYName>((odb::query<model::PYName>::parent == pyname.id && odb::query<model::PYName>::type == "baseclass"));
        const std::vector<std::uint64_t> bases_refs = PythonServiceHandler::transformReferences(std::vector<model::PYName>(bases.begin(), bases.end()), model::REF_ID);

        nodes = _db->query<model::PYName>((odb::query<model::PYName>::ref_id.in_range(bases_refs.begin(), bases_refs.end()) && odb::query<model::PYName>::is_definition == true && odb::query<model::PYName>::is_import == false) + order_by);
        break;
    }

    return std::vector<model::PYName>(nodes.begin(), nodes.end());
  });
}

std::vector<model::PYName> PythonServiceHandler::queryNodesInFile(const core::FileId& fileId, bool definitions)
{
  return _transaction([&](){
    const odb::query<model::PYName> order_by = "ORDER BY" + odb::query<model::PYName>::line_start + "," + odb::query<model::PYName>::column_start;
    odb::result<model::PYName> nodes = (definitions) ?
      _db->query<model::PYName>((odb::query<model::PYName>::file_id == std::stoull(fileId) && odb::query<model::PYName>::is_definition == true && odb::query<model::PYName>::is_import == false) + order_by)
      : _db->query<model::PYName>((odb::query<model::PYName>::file_id == std::stoull(fileId)) + order_by);

    return std::vector<model::PYName>(nodes.begin(), nodes.end());
  });
}

std::vector<std::uint64_t> PythonServiceHandler::transformReferences(const std::vector<model::PYName>& references, const model::PYNameID& id)
{
  std::vector<std::uint64_t> ret(references.size());
  switch(id)
  {
    case model::ID:
      std::transform(references.begin(), references.end(), ret.begin(), [](const model::PYName& e){return e.id;});
      break;
    case model::REF_ID:
      std::transform(references.begin(), references.end(), ret.begin(), [](const model::PYName& e){return e.ref_id;});
      break;
    case model::PARENT:
      std::transform(references.begin(), references.end(), ret.begin(), [](const model::PYName& e){return e.parent;});
      break;
    case model::PARENT_FUNCTION:
      std::transform(references.begin(), references.end(), ret.begin(), [](const model::PYName& e){return e.parent_function;});
      break;
  }
  return ret;
}

void PythonServiceHandler::setInfoProperties(AstNodeInfo& info, const model::PYName& pyname)
{
  info.id = std::to_string(pyname.id);
  info.astNodeValue = pyname.value;
  info.symbolType = pyname.type;
  info.range.file = std::to_string(pyname.file_id);
  info.range.range.startpos.line = pyname.line_start;
  info.range.range.startpos.column = pyname.column_start;
  info.range.range.endpos.line = pyname.line_end;
  info.range.range.endpos.column = pyname.column_end;
  info.astNodeType = pyname.type_hint;
}

std::string PythonServiceHandler::getNodeLineValue(const model::PYName& pyname)
{
  core::ProjectServiceHandler projectService(_db, _datadir, _context);
  std::string content;
  projectService.getFileContent(content, std::to_string(pyname.file_id));

  std::istringstream iss(content);
  std::string lineStr;

  for (std::size_t i = 1; i <= pyname.line_start; ++i)
  {
    std::getline(iss, lineStr);
  }

  return lineStr;
}
} // language
} // service
} // cc
