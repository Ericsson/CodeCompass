#ifndef CC_TEST_PYTHON_FILETESTER_H_
#define CC_TEST_PYTHON_FILETESTER_H_

#include "python_testhelper.h"


namespace cc
{
namespace test
{
namespace python
{

class PythonFileTester
{
public:
  PythonFileTester(PythonTestHelper helper_);

  // simple_test2
  bool checkAstNode_st2_highlight_variable(const std::string& filePathPrefix) const;
  bool checkAstNode_st2_highlight_function(const std::string& filePathPrefix) const;
  bool checkAstNodeContainerBinding_st2_attribute(const std::string& filePathPrefix) const;
  bool checkClassAttrNum_st2_attribute(const std::string& filePathPrefix) const;
  bool checkClassConstructor_st2_attribute(const std::string& filePathPrefix) const;
  bool checkClassBases_st2_attribute(const std::string& filePathPrefix) const;
  bool checkFunctionDefParams_st2_function(const std::string& filePathPrefix) const;
  bool checkFunctionDefDecorators_st2_function(const std::string& filePathPrefix) const;
  bool checkFunctionDefVariables_st2_function(const std::string& filePathPrefix) const;
  bool checkUnknownNameAndTarget_st2_unknown_attr(const std::string& filePathPrefix) const;
  bool checkUnknownNameAndTarget_st2_unknown_func(const std::string& filePathPrefix) const;

private:
  PythonTestHelper _helper;
};

} // python
} // test
} // cc

#endif // CC_TEST_PYTHON_FILETESTER_H_