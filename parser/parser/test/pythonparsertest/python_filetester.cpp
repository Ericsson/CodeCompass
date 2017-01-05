#include "python_filetester.h"

#include <iostream>
#include <memory>

namespace cc
{
namespace test
{
namespace python
{
PythonFileTester::PythonFileTester(PythonTestHelper helper_)
  : _helper(helper_)
{
}

bool PythonFileTester::checkAstNode_st2_highlight_variable(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "highlight/highlight_variable.py";
  bool result = false;

  result =
    _helper.checkAstNode("var", file, 6, 1) &&
    _helper.checkAstNode("L", file, 8, 5) &&
    _helper.checkAstNode("var", file, 11, 3) &&
    _helper.checkAstNode("var", file, 12, 9) &&
    _helper.checkAstNode("var", file, 14, 3) &&
    _helper.checkAstNode("var", file, 15, 6) &&
    _helper.checkAstNode("var", file, 16, 11) &&
    _helper.checkAstNode("E", file, 19, 5) &&
    _helper.checkAstNode("var", file, 22, 3) &&
    _helper.checkAstNode("inner", file, 24, 7) &&
    _helper.checkAstNode("var", file, 25, 11) &&
    _helper.checkAstNode("LE", file, 28, 5) &&
    _helper.checkAstNode("var", file, 31, 3) &&
    _helper.checkAstNode("inner", file, 33, 7) &&
    _helper.checkAstNode("var", file, 34, 5) &&
    _helper.checkAstNode("var", file, 35, 8) &&
    _helper.checkAstNode("var", file, 36, 13) &&
    _helper.checkAstNode("G", file, 39, 5) &&
    _helper.checkAstNode("var", file, 42, 9) &&
    _helper.checkAstNode("var", file, 44, 6) &&
    _helper.checkAstNode("var", file, 45, 11) &&
    _helper.checkAstNode("EG", file, 48, 5) &&
    _helper.checkAstNode("inner", file, 51, 7) &&
    _helper.checkAstNode("var", file, 53, 8) &&
    _helper.checkAstNode("var", file, 54, 13) &&
    _helper.checkAstNode("GWithAssigment", file, 57, 5) &&
    _helper.checkAstNode("var", file, 62, 3) &&
    _helper.checkAstNode("var", file, 63, 9) &&
    _helper.checkAstNode("var", file, 65, 3) &&
    _helper.checkAstNode("var", file, 66, 6) &&
    _helper.checkAstNode("var", file, 67, 11) &&
    _helper.checkAstNode("EGWithAssigment", file, 70, 5) &&
    _helper.checkAstNode("inner", file, 73, 7) &&
    _helper.checkAstNode("var", file, 76, 5) &&
    _helper.checkAstNode("var", file, 77, 11) &&
    _helper.checkAstNode("var", file, 79, 5) &&
    _helper.checkAstNode("var", file, 80, 8) &&
    _helper.checkAstNode("var", file, 81, 13);

   return result;
}

 bool PythonFileTester::checkAstNode_st2_highlight_function(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "highlight/highlight_function.py";
  bool result = false;

  result =
    _helper.checkAstNode("func", file, 6, 5) &&
    _helper.checkAstNode("param", file, 10, 16) &&
    _helper.checkAstNode("funcParam1", file, 10, 5) &&
    _helper.checkAstNode("param2", file, 14, 24) &&
    _helper.checkAstNode("funcParam2", file, 14, 5) &&
    _helper.checkAstNode("param1", file, 14, 16) &&
    _helper.checkAstNode("func", file, 18, 11) &&
    _helper.checkAstNode("funcPtr", file, 18, 1) &&
    _helper.checkAstNode("funcPtr", file, 19, 1) &&
    _helper.checkAstNode("funcParam1", file, 21, 11) &&
    _helper.checkAstNode("funcPtr", file, 21, 1) &&
    _helper.checkAstNode("funcPtr", file, 22, 1) &&
    _helper.checkAstNode("func", file, 23, 9) &&
    _helper.checkAstNode("funcPtr", file, 23, 1) &&
    _helper.checkAstNode("func", file, 26, 12) &&
    _helper.checkAstNode("funcParam2", file, 26, 1) &&
    _helper.checkAstNode("funcParam1", file, 26, 18) &&
    _helper.checkAstNode("FuncStorager", file, 29, 7) &&
    _helper.checkAstNode("memberFunc", file, 30, 7) &&
    _helper.checkAstNode("memberFuncParam1", file, 33, 7) &&
    _helper.checkAstNode("param", file, 33, 24) &&
    _helper.checkAstNode("param1", file, 37, 24) &&
    _helper.checkAstNode("memberFuncParam2", file, 37, 7) &&
    _helper.checkAstNode("param2", file, 37, 32) &&
    _helper.checkAstNode("fs", file, 40, 1) &&
    _helper.checkAstNode("FuncStorager", file, 40, 6) &&
    _helper.checkAstNode("fs", file, 41, 1) &&
    _helper.checkAstNode("memberFunc", file, 41, 4) &&
    _helper.checkAstNode("funcParam1", file, 42, 21) &&
    _helper.checkAstNode("fs", file, 42, 1) &&
    _helper.checkAstNode("memberFuncParam1", file, 42, 4) &&
    _helper.checkAstNode("fs", file, 44, 12) &&
    _helper.checkAstNode("memberFuncParam2", file, 44, 36) &&
    _helper.checkAstNode("fs", file, 44, 33) &&
    _helper.checkAstNode("memberFuncParam1", file, 44, 15) &&
    _helper.checkAstNode("funcParam2", file, 44, 1);

   return result;
}

bool PythonFileTester::checkAstNodeContainerBinding_st2_attribute(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "attributes/attribute.py";
  bool result = false;

  result =
    _helper.checkAstNodeContainerBinding("attribute", "A", file, 1) &&
    _helper.checkAstNodeContainerBinding("B", "b_attr1", file, 6) &&
    _helper.checkAstNodeContainerBinding("B", "__init__", file, 8) &&
    _helper.checkAstNodeContainerBinding("__init__", "self", file, 8) &&
    _helper.checkAstNodeContainerBinding("__init__", "b_attr1", file, 9)&&
    _helper.checkAstNodeContainerBinding("foo", "A", file, 14);

   return result;
}

bool PythonFileTester::checkClassAttrNum_st2_attribute(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "attributes/attribute.py";
  bool result = false;

  result =
    _helper.checkClassAttrNum(3, "B", file, 4);

   return result;
}

bool PythonFileTester::checkClassConstructor_st2_attribute(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "attributes/attribute.py";
  bool result = false;

  result =
    _helper.checkClassConstructor(8, "B", file, 4);

   return result;
}

bool PythonFileTester::checkClassBases_st2_attribute(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "attributes/attribute.py";
  bool result = false;

  result =
    _helper.checkClassBases(1, "B", file, 4);

   return result;
}

bool PythonFileTester::checkFunctionDefParams_st2_function(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "functions/function.py";
  bool result = false;

  result =
    _helper.checkFunctionDefParams(2, "testParams", file, 1) &&
    _helper.checkFunctionDefParams(2, "testVararg", file, 5) &&
    _helper.checkFunctionDefParams(1, "some_static_method", file, 19);

   return result;
}

bool PythonFileTester::checkFunctionDefDecorators_st2_function(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "functions/function.py";
  bool result = false;

  result =
    _helper.checkFunctionDefDecorators("classmethod", "some_class_method", file, 15) &&
    _helper.checkFunctionDefDecorators("staticmethod", "some_static_method", file, 19);

   return result;
}

bool PythonFileTester::checkFunctionDefVariables_st2_function(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "functions/function.py";
  bool result = false;

  result =
    _helper.checkFunctionDefVariables(3, "funcWithLocVars", file, 22);

   return result;
}

bool PythonFileTester::checkUnknownNameAndTarget_st2_unknown_attr(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "unknowns/unknown_attr.py";
  bool result = false;

  result =
    _helper.checkUnknownNameAndTarget("b", "c", file, 1) &&
    _helper.checkUnknownNameAndTarget("b.c", "d", file, 1);

   return result;
}

bool PythonFileTester::checkUnknownNameAndTarget_st2_unknown_func(const std::string& filePathPrefix) const
{
  std::string file = filePathPrefix + "unknowns/unknown_func.py";
  bool result = false;

  result =
    _helper.checkUnknownNameAndTarget("b", "c", file, 1) &&
    _helper.checkUnknownNameAndTarget("b.c", "d", file, 1)&&
    _helper.checkUnknownNameAndTarget("b.c.d", "e", file, 1);

   return result;
}

} // python
} // test
} // cc
