#include "pythonparser/pythonparser.h"

#include <iostream>

#include <sstream>
#include <iterator>

#include <boost/algorithm/string.hpp>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/filesystem.h>
#include <util/environment.h>
#include <util/odbtransaction.h>
#include <model/filecontent.h>
#include <parser/sourcemanager.h>

#include <model/buildlog.h>
#include <model/buildlog-odb.hxx>

#include <model/python/pythonastnode.h>
#include <model/python/pythonastnode-odb.hxx>

#include <model/python/pythonbinding.h>
#include <model/python/pythonbinding-odb.hxx>

# include <model/python/pythonastnode.h>
# include <model/python/pythonastnode-odb.hxx>

# include <model/python/pythonattribute.h>
# include <model/python/pythonattribute-odb.hxx>

# include <model/python/pythonbinding.h>
# include <model/python/pythonbinding-odb.hxx>

# include <model/python/pythonclassdef.h>
# include <model/python/pythonclassdef-odb.hxx>

# include <model/python/pythondecorator.h>
# include <model/python/pythondecorator-odb.hxx>

# include <model/python/pythonfunctiondef.h>
# include <model/python/pythonfunctiondef-odb.hxx>

# include <model/python/pythonfunctioncall.h>
# include <model/python/pythonfunctioncall-odb.hxx>

# include <model/python/pythonfunctionparam.h>
# include <model/python/pythonfunctionparam-odb.hxx>

# include <model/python/pythoninheritance.h>
# include <model/python/pythoninheritance-odb.hxx>

# include <model/python/pythonreference.h>
# include <model/python/pythonreference-odb.hxx>

# include <model/python/pythonunknown.h>
# include <model/python/pythonunknown-odb.hxx>

# include <model/python/pythonvariable.h>
# include <model/python/pythonvariable-odb.hxx>

# include <model/python/pythonvariableref.h>
# include <model/python/pythonvariableref-odb.hxx>

# include <model/python/pythonviews.h>
# include <model/python/pythonviews-odb.hxx>


namespace cc
{
namespace parser
{

model::FileLoc PythonParser::convertFileLoc(const TFileLoc& rhs_) const
{
  model::FileLoc ret;

  model::FilePtr file;
  if (!_srcMgr->getCreateFile(rhs_.file, file))
  {
    throw std::runtime_error(
      "File with path: " + rhs_.file + "is not exist.");
  }

  ret.file = file;
  ret.range.start.line = rhs_.lineStart;
  ret.range.start.column = rhs_.offset;
  ret.range.end.line = rhs_.lineEnd;
  ret.range.end.column = rhs_.endOffset;

  return ret;
}

odb::nullable<uint64_t> PythonParser::convertNullableWithHash(
  const std::string& rhs_) const
{
  return (rhs_ == "null" || rhs_ == "" )
    ? odb::nullable<uint64_t>()
    : odb::nullable<uint64_t>(util::fnvHash(rhs_));
}

odb::nullable<std::string> PythonParser::convertNullable(
  const std::string& rhs_) const
{
  return (rhs_ == "null" || rhs_ == "" )
    ? odb::nullable<std::string>()
    : odb::nullable<std::string>(rhs_);
}

PythonParser::PythonParser(std::shared_ptr<model::Workspace> ws_) :
  _workspace(ws_),
  _srcMgr(nullptr)
{}

void PythonParser::beforeTraverse(
    const OptionMap& projectOptions_,
    SourceManager& srcMgr_)
{
  _projectOptions = projectOptions_;
}

void PythonParser::afterTraverse(SourceManager& srcMgr_)
{
  if (_roots.empty())
  {
    return;
  }

  SLog(util::DEBUG) << "PythonParser afterTraverse in";

  util::OdbTransaction transaction(std::shared_ptr<odb::database>(
    _workspace->getDb(), util::NoDelete()));

  _srcMgr = &srcMgr_;

  transaction([&, this]() {

    _parserProc.reset(new PytonhParserProcess(_roots, _projectOptions));

    auto ppsif = std::static_pointer_cast<PythonPersisterServiceIf>(
      shared_from_this());
    _parserProc->startServe(util::make_shared_ptr(ppsif));
    _parserProc.reset();

    // ------------------------ FULLY PARSED ----------------------------------
    typedef odb::query<model::File> FileQuery;

    odb::result<model::File> fileResult = _workspace->getDb()->query<model::File>(
      FileQuery::path.like("%.py") &&
      FileQuery::parseStatus != model::File::ParseStatus::PSPartiallyParsed);
    for (auto f : fileResult)
    {
      f.parseStatus = model::File::ParseStatus::PSFullyParsed;
      _workspace->getDb()->update(f);
    }
    // ------------------------------------------------------------------------
  });

  _srcMgr = nullptr;

  SLog(util::DEBUG) << "PythonParser afterTraverse out";
}

Traversal::DirIterCallback PythonParser::traverse(
  const std::string& path_,
  SourceManager&)
{
  SLog(util::DEBUG) << "PythonParser adding " << path_;

  _roots.push_back(path_);
  return Traversal::DirIterCallback();
}

void PythonParser::stop()
{
  _parserProc->stopServe();
}


//
// Persister methods
//

void PythonParser::addNode(const TPythonAstNode& node_)
{
  model::PythonAstNode node;

  node.id = util::fnvHash(node_.id);
  node.name = node_.name;
  node.abv_qname = node_.abv_qname;
  node.ast_type =
    static_cast<model::PythonAstNode::AstType>(node_.astType);
  node.location = convertFileLoc(node_.location);


  node.base_binding = convertNullableWithHash(node_.baseBinding);
  node.container_binding = convertNullableWithHash(node_.containerBinding);
  node.global_write = node_.globalWrite;
  _workspace->getDb()->persist(node);
}

void PythonParser::addBinding(const TPythonBinding& binding_)
{
  model::PythonBinding binding;

  binding.id = util::fnvHash(binding_.id);
  binding.name = binding_.name;
  binding.formatted_qname = convertNullable(binding_.formattedQname);
  binding.mangled_name = convertNullable(binding_.mangledName);
  binding.kind = static_cast<model::PythonBinding::Kind>(
    binding_.kind);
  binding.type = binding_.type;
  binding.location = convertFileLoc(binding_.location);
  binding.documentation = binding_.documentation;

  _workspace->getDb()->persist(binding);
}

void PythonParser::addReference(const TPythonReference& reference_)
{
  model::PythonReference reference;

  reference.node = util::fnvHash(reference_.node);
  reference.binding = util::fnvHash(reference_.binding);

  _workspace->getDb()->persist(reference);
}

void PythonParser::addUnknown(const TPythonUnknown& unknown_)
{
  model::PythonUnknown unknown;

  unknown.id = util::fnvHash(unknown_.id);
  unknown.target = convertNullable(unknown_.target);
  unknown.name = unknown_.name;
  unknown.kind = static_cast<model::PythonUnknown::Kind>(unknown_.kind);

  _workspace->getDb()->persist(unknown);
}

void PythonParser::addClassDef(const TPythonClassDef& classDef_)
{
  model::PythonClassDef classDef;

  classDef.target = util::fnvHash(classDef_.target);
  classDef.constructor = convertNullableWithHash(classDef_.constructor);

  _workspace->getDb()->persist(classDef);
}

void PythonParser::addFunctionDef(const TPythonFunctionDef& functionDef_)
{
  model::PythonFunctionDef functionDef;

  functionDef.id = util::fnvHash(functionDef_.id);
  functionDef.member_of = convertNullableWithHash(functionDef_.memberOf);
  functionDef.min_param_num = functionDef_.minParamNum;
  functionDef.max_param_num = functionDef_.maxParamNum;

  _workspace->getDb()->persist(functionDef);
}

void PythonParser::addFunctionCall(const TPythonFunctionCall& functionCall_)
{
  model::PythonFunctionCall functionCall;

  functionCall.id = util::fnvHash(functionCall_.id);
  functionCall.arg_num = functionCall_.argNum;

  _workspace->getDb()->persist(functionCall);
}

void PythonParser::addFunctionParam(const TPythonFunctionParam& functionParam_)
{
  model::PythonFunctionParam functionParam;

  functionParam.id = util::fnvHash(functionParam_.id);
  functionParam.target = util::fnvHash(functionParam_.target);

  _workspace->getDb()->persist(functionParam);
}

void PythonParser::addDecorator(const TPythonDecorator& decorator_)
{
  model::PythonDecorator decorator;

  decorator.id = util::fnvHash(decorator_.id);
  decorator.target = util::fnvHash(decorator_.target);
  decorator.value = decorator_.value;

  _workspace->getDb()->persist(decorator);
}

void PythonParser::addInheritance(const TPythonInheritance& inheritance_)
{
  model::PythonInheritance inheritance;

  inheritance.target = util::fnvHash(inheritance_.target);
  inheritance.base = util::fnvHash(inheritance_.base);
  inheritance.kind = static_cast<model::PythonInheritance::Kind>(
    inheritance_.kind);

  _workspace->getDb()->persist(inheritance);
}

void PythonParser::addAttribute(const TPythonAttribute& attributes_)
{
  model::PythonAttribute attributes;

  attributes.attribute = util::fnvHash(attributes_.attribute);
  attributes.target = convertNullableWithHash(attributes_.target);

  _workspace->getDb()->persist(attributes);
}

void PythonParser::addVariable(const TPythonVariable& variable_)
{
  model::PythonVariable variable;

  variable.id = util::fnvHash(variable_.id);
  variable.target = util::fnvHash(variable_.target);
  variable.is_global = variable_.isGlobal;

  _workspace->getDb()->persist(variable);
}

void PythonParser::addVariableRef(const TPythonVariableRef& vref_)
{
  model::PythonVariableRef vref;

  vref.astNode.reset(*(_workspace->getDb()), util::fnvHash(vref_.nodeId));
  vref.mangledName = vref_.mangledName;

  switch (vref_.kind)
  {
    case TPythonVariableRefKind::Definition:
      vref.refType = model::PythonVariableRef::RefType::Definition;
      break;
    case TPythonVariableRefKind::Read:
      vref.refType = model::PythonVariableRef::RefType::Read;
      break;
    case TPythonVariableRefKind::Write:
      vref.refType = model::PythonVariableRef::RefType::Write;
      break;
  }

  _workspace->getDb()->persist(vref);
}

void PythonParser::failedToParse(const std::string& filePath_)
{
  typedef odb::query<model::File> FileQuery;

  odb::result<model::File> fileResult = _workspace->getDb()->query<model::File>(
    FileQuery::path == filePath_);
  for (auto f : fileResult)
  {
    f.parseStatus = model::File::ParseStatus::PSPartiallyParsed;
    // f.parseStatus = model::File::ParseStatus::PSNone;
    _workspace->getDb()->update(f);
  }
}


} // parser
} // cc
