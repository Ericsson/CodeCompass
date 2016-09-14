#ifndef CC_TEST_PYTHON_PYTHONTESTHELPER_H_
#define CC_TEST_PYTHON_PYTHONTESTHELPER_H_

#include <model/workspace.h>
#include <util/odbtransaction.h>

#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/query-dynamic.hxx>
#include <odb/result.hxx>
#include <odb/core.hxx>

#include <model/fileloc.h>

# include <model/python/pythonastnode-odb.hxx>
# include <model/python/pythonattribute-odb.hxx>
# include <model/python/pythonbinding-odb.hxx>
# include <model/python/pythonclassdef-odb.hxx>
# include <model/python/pythondecorator-odb.hxx>
# include <model/python/pythonfunctiondef-odb.hxx>
# include <model/python/pythonfunctionparam-odb.hxx>
# include <model/python/pythoninheritance-odb.hxx>
# include <model/python/pythonreference-odb.hxx>
# include <model/python/pythonunknown-odb.hxx>
# include <model/python/pythonvariable-odb.hxx>
# include <model/python/pythonviews-odb.hxx>


namespace cc
{
namespace service
{
namespace core
{
struct FilePosition;
} // core
} // service
} // cc

namespace cc
{
namespace test
{
namespace python
{

class PythonTestHelper
{

typedef odb::query<model::PythonAstNode> queryAstNode;
typedef odb::query<model::PythonAttribute> queryAttribute;
typedef odb::query<model::PythonBinding> queryBinding;
typedef odb::query<model::PythonClassDef> queryClassDef;
typedef odb::query<model::PythonDecorator> queryDecorator;
typedef odb::query<model::PythonFunctionDef> queryFunctionDef;
typedef odb::query<model::PythonFunctionParam> queryFunctionParam;
typedef odb::query<model::PythonInheritance> queryInheritance;
typedef odb::query<model::PythonReference> queryReference;
typedef odb::query<model::PythonUnknown> queryUnknown;
typedef odb::query<model::PythonVariable> queryVariable;

typedef model::PythonAstNode astNode;


public:
  PythonTestHelper(odb::database& db_);

  bool checkAstNode(
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkAstNodeContainerBinding(
    const std::string& container_name_,
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkBindingType(
    const std::string& type_,
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkBindingRefNum(
    const int expectedRefNum_,
    const bool thisFileOnly_,
    const std::string& base_node_name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkClassAttrNum(
    const int expectedAttrNum_,
    const std::string& className_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkClassConstructor(
    const unsigned int expectedConstrStartLine_,
    const std::string& className_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkClassBases(
    const int expectedBaseNumber_,
    const std::string& className_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkFunctionDefParams(
    const int expectedParamNumber_,
    const std::string& funcName_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkFunctionDefDecorators(
    const std::string& expectedDecoratorValue_,
    const std::string& funcName_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkFunctionDefVariables(
    const int expectedVariableNum_,
    const std::string& funcName_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;
  
  bool checkUnknownNameAndTarget(
    const std::string& expectedUnknownTarget_,
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;

private:
  model::PythonAstNode getAstNodeByPosition(
    const std::string& name_,
    const std::string& filename_,
    const int startLine_,
    const int offset_ = 0
    ) const;

  odb::database& _db;
};

} // python
} // test
} // cc

#endif // CC_TEST_PYTHON_PYTHONTESTHELPER_H_