#include "python_testhelper.h"

#include <iostream>
#include <memory>

#include <stdexcept>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <model/workspace.h>

#include <parser/parser.h>
#include <pythonparser/pythonparser.h>
#include <parser/projectparser.h>
#include <projectparser/generalprojectparser.h>
#include <projectparser/xmlprojectparser.h>

#include <util/streamlog.h>



namespace cc
{
namespace test
{
namespace python
{

class InvalidPos : public std::runtime_error
{
public:
  InvalidPos() : std::runtime_error("Invalid position!") {}
};


PythonTestHelper::PythonTestHelper(odb::database& db_)
  : _db(db_)
{
}

model::PythonAstNode PythonTestHelper::getAstNodeByPosition(
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_
    ) const
{
  // std::cout << "name_: " << name_ << std::endl;
  // std::cout << "filename_: " << filename_ << std::endl;
  // std::cout << "startLine_: " << startLine_ << std::endl;
  // std::cout << "offset_: " << offset_ << std::endl;

  odb::result<model::PythonAstNode> nodeResult;

  nodeResult = (offset_ < 1) ?
               _db.query<model::PythonAstNode>(
                 queryAstNode::location.file->path == filename_
                 &&
                 queryAstNode::location.range.start.line == startLine_
                 &&
                 queryAstNode::name == name_
                 &&
                 queryAstNode::ast_type != model::PythonAstNode::AstType::Module
               )
               :
               _db.query<model::PythonAstNode>(
                 queryAstNode::location.file->path == filename_
                 &&
                 queryAstNode::location.range.start.line == startLine_
                 &&
                 queryAstNode::location.range.start.column == offset_
                 &&
                 queryAstNode::name == name_
                 &&
                 queryAstNode::ast_type != model::PythonAstNode::AstType::Module
               );

  if (nodeResult.empty())
  {
    std::cout << "\n...No node found. Exception...";
    throw InvalidPos();
  }

  return *(nodeResult.begin());
}



bool PythonTestHelper::checkAstNode(
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_
    ) const
{
  astNode n;
  try
  {
    n = getAstNodeByPosition(name_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  return (n.name == name_);
}


// node shoud be a base of a binding
bool PythonTestHelper::checkBindingType(
    const std::string& type_,
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(name_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto binding = _db.find<model::PythonBinding>(node.id);
  if (!binding)
    return false;

  return (binding->type.find(type_) != std::string::npos);
}



bool PythonTestHelper::checkAstNodeContainerBinding(
    const std::string& container_name_,
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(name_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  if(node.container_binding.null())
    return false;

  auto container_binding = _db.find<model::PythonBinding>(
    *node.container_binding);
  if(!container_binding)
    return false;

  return (container_binding->name == container_name_);
}

// node shoud be a base of a binding
bool PythonTestHelper::checkBindingRefNum(
    const int expectedRefNum_,
    const bool thisFileOnly_,
    const std::string& base_node_name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(base_node_name_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto allRefResult = _db.query<model::PythonReference>(
    queryReference::binding == node.id);

  int size = (thisFileOnly_)
    ? std::count_if(allRefResult.begin(), allRefResult.end(),
        [this, &filename_](const model::PythonReference& r)
        {
          auto refered_node = _db.find<model::PythonAstNode>(r.node);
          if(refered_node &&
            refered_node->location.file.load()->filename == filename_)
            return true;

          return false;
        })
    : std::count_if(allRefResult.begin(), allRefResult.end(),
        [](const model::PythonReference& r) { return true; });

  return size == expectedRefNum_;
}

bool PythonTestHelper::checkClassAttrNum(
  const int expectedAttrNum_,
  const std::string& className_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(className_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto allAttrResult = _db.query<model::PythonAttribute>(
    queryAttribute::target == node.id);

  int size = std::count_if(allAttrResult.begin(), allAttrResult.end(),
        [](const model::PythonAttribute&) { return true; });

  return size == expectedAttrNum_;
}
bool PythonTestHelper::checkClassConstructor(
  const unsigned int expectedConstrStartLine_,
  const std::string& className_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(className_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto classDefResult = _db.query<model::PythonClassDef>(
    queryClassDef::id == node.id);

  for(auto c : classDefResult)
  {
    auto constructorResult = _db.query<model::PythonAstNode>(
      queryAstNode::id == c.constructor);

    if(!constructorResult.empty())
    {
      return ((*(constructorResult.begin())).location.range.start.line // ???????????????
               == expectedConstrStartLine_);
    }
  }
  return false;
}
bool PythonTestHelper::checkClassBases(
  const int expectedBaseNumber_,
  const std::string& className_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(className_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto basesResult = _db.query<model::PythonInheritance>(
    queryInheritance::target == node.id);

  int size =  std::count_if(basesResult.begin(), basesResult.end(),
        [](const model::PythonInheritance&) { return true; });

  return size == expectedBaseNumber_;
}

bool PythonTestHelper::checkFunctionDefParams(
  const int expectedParamNumber_,
  const std::string& funcName_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(funcName_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto paramsResult = _db.query<model::PythonFunctionParam>(
    queryFunctionParam::target == node.id);

  int size =  std::count_if(paramsResult.begin(), paramsResult.end(),
        [](const model::PythonFunctionParam&) { return true; });

  return size == expectedParamNumber_;
}
bool PythonTestHelper::checkFunctionDefDecorators(
  const std::string& expectedDecoratorValue_,
  const std::string& funcName_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(funcName_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  auto decoratorResult = _db.query<model::PythonDecorator>(
    queryDecorator::target == node.id);

  for(auto d : decoratorResult){
    if(d.value == expectedDecoratorValue_){
      return true;
    }
  }
  return false;
}
bool PythonTestHelper::checkFunctionDefVariables(
  const int expectedVariableNum_,
  const std::string& funcName_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  astNode node;
  try
  {
    node = getAstNodeByPosition(funcName_, filename_, startLine_, offset_);
  }
  catch (InvalidPos &ex)
  {
    std::cout << "\nInvalidPosition Exception at getAstNodeByPosition()...\n";
    return false;
  }

  std::cout << "nodeid: " << node.id << std::endl;

  auto localVarResult = _db.query<model::PythonAstNode>(
    queryAstNode::container_binding == node.id &&
    queryAstNode::ast_type == model::PythonAstNode::AstType::Variable);

  int size =  std::count_if(localVarResult.begin(), localVarResult.end(),
        [](const model::PythonAstNode& n) {
          if(!n.base_binding.null()){
            std::cout << "YEAH";
            return !n.base_binding.null();
          }
        });

  return size == expectedVariableNum_;
}
bool PythonTestHelper::checkUnknownNameAndTarget(
  const std::string& expectedUnknownTarget_,
  const std::string& name_,
  const std::string& filename_,
  const int startLine_,
  const int offset_
    ) const
{
  auto unknownResult = _db.query<model::PythonUnknown>(
    queryUnknown::target == expectedUnknownTarget_ &&
    queryUnknown::name == name_);

  return !unknownResult.empty();
}

} // python
} // test
} // cc
