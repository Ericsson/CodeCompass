#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppenum.h>
#include <model/cppenum-odb.hxx>
#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/cppnamespace.h>
#include <model/cppnamespace-odb.hxx>
#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>
#include <model/cppvariable.h>
#include <model/cppvariable-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

extern const char* dbConnectionString;

using namespace cc;

using QCppAstNode = odb::query<model::CppAstNode>;
using QCppFunction = odb::query<model::CppFunction>;
using QCppEnum = odb::query<model::CppEnum>;
using QCppEnumConstant = odb::query<model::CppEnumConstant>;
using QCppNamespace = odb::query<model::CppNamespace>;
using QCppRecord = odb::query<model::CppRecord>;
using QCppVariable = odb::query<model::CppVariable>;
using QFile = odb::query<model::File>;
using RCppAstNode = odb::result<model::CppAstNode>;

class CppParserTest : public ::testing::Test
{
public:
  CppParserTest() :
    _db(cc::util::connectDatabase(dbConnectionString)),
    _transaction(_db)
  {

  }

protected:
  std::shared_ptr<odb::database> _db;
  cc::util::OdbTransaction _transaction;
};

TEST_F(CppParserTest, FilesAreInDatabase)
{
  _transaction([&, this]() {
    model::File file;

    file = _db->query_value<model::File>(QFile::filename == "cxxrecord.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(QFile::filename == "enum.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(QFile::filename == "function.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(QFile::filename == "typedef.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(QFile::filename == "variable.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(QFile::filename == "namespace.cpp");
    EXPECT_EQ(file.type, "CPP");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);
  });
}

TEST_F(CppParserTest, SimpleFunction)
{
  _transaction([&, this]() {
    model::File simpleFile = _db->query_value<model::File>(
      QFile::filename == "function.cpp");

    model::CppFunction simpleFunc = _db->query_value<model::CppFunction>(
      QCppFunction::name == "singleFunc");
    model::CppAstNode simpleFuncAstNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == simpleFunc.entityHash);

    EXPECT_EQ(simpleFuncAstNode.astValue, "void singleFunc()");
    EXPECT_EQ(
      simpleFuncAstNode.symbolType,
      model::CppAstNode::SymbolType::Function);
    EXPECT_EQ(
      simpleFuncAstNode.astType,
      model::CppAstNode::AstType::Definition);
    EXPECT_TRUE(simpleFuncAstNode.visibleInSourceCode);

    EXPECT_EQ(simpleFuncAstNode.location.file.object_id(), simpleFile.id);
    EXPECT_EQ(simpleFuncAstNode.location.range.start.line, 1);
    EXPECT_EQ(simpleFuncAstNode.location.range.start.column, 1);
    EXPECT_EQ(simpleFuncAstNode.location.range.end.line, 4);
    EXPECT_EQ(simpleFuncAstNode.location.range.end.column, 2);

    model::CppFunction simpleFuncFunc = _db->query_value<model::CppFunction>(
      odb::query<model::CppFunction>::qualifiedName == "singleFunc");

    EXPECT_EQ(simpleFuncFunc.name, "singleFunc");
    EXPECT_EQ(simpleFuncFunc.qualifiedName, "singleFunc");
  });
}

TEST_F(CppParserTest, FunctionDeclarationOnly)
{
  _transaction([&, this]() {
    model::CppFunction funcDecl = _db->query_value<model::CppFunction>(
      QCppFunction::name == "funcDecl");
    model::CppAstNode funcDeclAstNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == funcDecl.entityHash);

    EXPECT_EQ(
      funcDeclAstNode.symbolType,
      model::CppAstNode::SymbolType::Function);
    EXPECT_EQ(
      funcDeclAstNode.astType,
      model::CppAstNode::AstType::Declaration);
  });
}

TEST_F(CppParserTest, FunctionWithMultipleDeclarations)
{
  _transaction([&, this]() {
    model::CppFunction callee = _db->query_value<model::CppFunction>(
      QCppFunction::name == "multiFunction");
    RCppAstNode multiFuncAstNode = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == callee.entityHash);

    int numDecl = 0, numDef = 0, numOther = 0;
    for (const model::CppAstNode& n : multiFuncAstNode)
      switch (n.astType)
      {
        case model::CppAstNode::AstType::Declaration: ++numDecl; break;
        case model::CppAstNode::AstType::Definition: ++numDef; break;
        default: ++numOther; break;
      }

    EXPECT_EQ(numDecl, 2);
    EXPECT_EQ(numDef, 1);
    EXPECT_EQ(numOther, 0);
  });
}

TEST_F(CppParserTest, FunctionCall)
{
  _transaction([&, this]() {
    model::CppFunction callee = _db->query_value<model::CppFunction>(
      QCppFunction::name == "callee");
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == callee.entityHash);

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case 12:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 20:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Usage);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue == "MyClass::MyClass()" &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    model::FileId fileId = astNodes.begin()->location.file.object_id();

    int numDefault = 0, numMove = 0, numCopy = 0;

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_TRUE(
        n.location.range.start.line == 10 || // Global variable.
        n.location.range.start.line == 15 || // Local variable.
        n.location.range.start.line == 20 || // Temporary object.
        n.location.range.start.line == 34 || // Parent constructor.
        n.location.range.start.line == 35 || // Temporary object.
        n.location.range.start.line == 38 || // New object.
        n.location.range.start.line == 42);  // New object.
      ++numDefault;
    }

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue == "MyClass::MyClass(MyClass &&)" &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_TRUE(
        n.location.range.start.line == 20 || // Throwing an exception.
        n.location.range.start.line == 25 || // Returning a value.
        n.location.range.start.line == 35);  // Initializing a field.
      ++numMove;
    }

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue == "MyClass::MyClass(const MyClass &)" &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_TRUE(
        n.location.range.start.line == 16 || // Local variable.
        n.location.range.start.line == 17);  // Local variable.
      ++numCopy;
    }

    EXPECT_EQ(numDefault, 7);
    EXPECT_EQ(numMove, 3);
    EXPECT_EQ(numCopy, 2);

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::location.file == fileId &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    for (const model::CppAstNode& n : astNodes)
      if (n.astValue == "void * operator new(unsigned long)")
        EXPECT_EQ(n.location.range.start.line, 38);
      else if (n.astValue == "void * operator new[](unsigned long)")
        EXPECT_EQ(n.location.range.start.line, 42);
      else if (n.astValue == "void operator delete(void *)")
        EXPECT_EQ(n.location.range.start.line, 41);
      else if (n.astValue == "void operator delete[](void *)")
        EXPECT_EQ(n.location.range.start.line, 43);

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue.like("%virtualFunction%"));

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case 5:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 39:
          EXPECT_TRUE(
            n.astType == model::CppAstNode::AstType::VirtualCall ||
            // TODO: Should be removed.
            n.astType == model::CppAstNode::AstType::Read);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue.like("%nonVirtualFunction%"));

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case 6:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 40:
          EXPECT_TRUE(
            n.astType == model::CppAstNode::AstType::Usage ||
            // TODO: Should be removed.
            n.astType == model::CppAstNode::AstType::Read);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }
  });
}

TEST_F(CppParserTest, Typedef)
{
  _transaction([&, this]() {
    model::CppEnum integer = _db->query_value<model::CppEnum>(
      QCppEnum::name == "Integer");
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == integer.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Typedef);

      switch (n.location.range.start.line)
      {
        case 1: // Typedef definition.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 3:  // Global variable.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::GlobalTypeLoc);
          break;

        case 5:  // Function return type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ReturnTypeLoc);
          break;

        case 6:  // Function parameter.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ParameterTypeLoc);
          break;

        case 8:  // Local variable.
        case 16: // Caught exception object.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::LocalTypeLoc);
          break;

        case 9:  // Temporary object.
        case 10: // Typedef type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypeLocation);
          break;

        default: // Nothing else is allowed.
          EXPECT_TRUE(false);
          break;
      }
    }
  });
}

TEST_F(CppParserTest, Record)
{
  _transaction([&, this] {
    model::CppRecord myClass = _db->query_value<model::CppRecord>(
      QCppRecord::name == "MyClass");
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == myClass.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Type);

      switch (n.location.range.start.line)
      {
        // case -1: 
        //   EXPECT_TRUE(
        //     // TODO: investigate the type of this. It is possibly the parameter
        //     // of a compiler generated copy constructor or assignment operator.
        //     // ParameterTypeLoc and ReturnTypeLoc are both listed here,
        //     // however, only one of them is found. Earlier when we generated
        //     // mangled names manually, the function parameter was stored, now
        //     // with USRs we have the return type.
        //     n.astType == model::CppAstNode::AstType::ParameterTypeLoc ||
        //     n.astType == model::CppAstNode::AstType::ReturnTypeLoc ||
        //     n.astType == model::CppAstNode::AstType::TypeLocation);
        //   break;

        case 1: // Forward declaration.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Declaration);
          break;

        case 2: // Class definition.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 10: // Global variable.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::GlobalTypeLoc);
          break;

        case 12: // Return type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ReturnTypeLoc);
          break;

        case 13: // Function parameter.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ParameterTypeLoc);
          break;

        case 15: // Local variable.
        case 16: // Local variable.
        case 17: // Local variable.
        case 22: // Exception catch.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::LocalTypeLoc);
          break;

        case 28: // Typedef type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypedefTypeLoc);
          break;

        case 30: // Base class in an inheritance.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::InheritanceTypeLoc);
          break;

        case 32: // Field of a class.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::FieldTypeLoc);
          break;

        case 33:
          // Return type in a function pointer type.
          if (n.location.range.start.column == 3)
            EXPECT_EQ(n.astType, model::CppAstNode::AstType::ReturnTypeLoc);
          // Parameter in a function pointer type.
          else if (n.location.range.start.column == 34)
            EXPECT_EQ(n.astType, model::CppAstNode::AstType::ParameterTypeLoc);
          else
            EXPECT_TRUE(false);
          break;

        case 20:
        case 35:
          // Temporary object.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypeLocation);
          break;

        case 38:
          // Local variable.
          if (n.location.range.start.column == 5)
            EXPECT_EQ(n.astType, model::CppAstNode::AstType::LocalTypeLoc);
          // Type name after "new" keyword.
          else if (n.location.range.start.column == 24)
            EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypeLocation);
          else
            EXPECT_TRUE(false);
          break;

        case 42: // Type name after "new[]" keyword.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypeLocation);
          break;

        default: // Nothing else is allowed.
          EXPECT_TRUE(false);
          break;
      }
    }
  });
}

TEST_F(CppParserTest, Enum)
{
  _transaction([&, this] {
    model::CppEnum enumeration = _db->query_value<model::CppEnum>(
      QCppEnum::name == "Enumeration");
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == enumeration.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Enum);

      switch (n.location.range.start.line)
      {
        case 1: // Enum declaration.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Declaration);
          break;

        case 2: // Enum definition.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 9:  // Global variable.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::GlobalTypeLoc);
          break;

        case 11: // Function return type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ReturnTypeLoc);
          break;

        case 12: // Function parameter type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::ParameterTypeLoc);
          break;

        case 14: // Local variable.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::LocalTypeLoc);
          break;

        case 15: // Temporary object.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypeLocation);
          break;

        case 16: // Typedef type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::TypedefTypeLoc);
          break;

        case 22: // Caught exception type.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::LocalTypeLoc);
          break;

        default: // Nothing else is allowed.
          break;
      }
    }

    model::CppEnumConstant first = _db->query_value<model::CppEnumConstant>(
      QCppEnumConstant::name == "First");
    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == first.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::EnumConstant);

      switch (n.location.range.start.line)
      {
        case 4: // Definition of an enum constant.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 14: // Usage of an enum constant.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Usage);
          break;

        default: // Nothing else is allowed.
          EXPECT_TRUE(false);
          break;
      }
    }

    model::CppFunction fieldFunction = _db->query_value<model::CppFunction>(
      QCppFunction::name == "fieldFunction");
    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == fieldFunction.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      switch (n.location.range.start.line)
      {
        case 33:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::FunctionPtr);
          break;

        case 36:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Write);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::FunctionPtr);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }
    }

    model::CppVariable fieldVariable = _db->query_value<model::CppVariable>(
      QCppVariable::name == "fieldVariable");
    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == fieldVariable.entityHash);

    for (const model::CppAstNode& n : astNodes)
    {
      switch (n.location.range.start.line)
      {
        case 32:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          break;

        case 35:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Write);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }
    }
  });
}

TEST_F(CppParserTest, Variable)
{
  _transaction([&, this] {
    model::CppVariable variableDefinition
      = _db->query_value<model::CppVariable>(
        QCppVariable::name == "variableDefinition");
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == variableDefinition.entityHash);

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case 8:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 13:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Read);
          break;

        case 14:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Write);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }

    model::CppVariable variableDeclaration
      = _db->query_value<model::CppVariable>(
        QCppVariable::name == "variableDeclaration");
    model::CppAstNode astNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == variableDeclaration.entityHash);

    EXPECT_EQ(astNode.location.range.start.line, 9);
    EXPECT_EQ(astNode.symbolType, model::CppAstNode::SymbolType::Variable);
    EXPECT_EQ(astNode.astType, model::CppAstNode::AstType::Declaration);

    model::CppVariable variableFunctionPointer
      = _db->query_value<model::CppVariable>(
        QCppVariable::name == "variableFunctionPointer");
    astNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == variableFunctionPointer.entityHash);

    EXPECT_EQ(astNode.location.range.start.line, 10);
    EXPECT_EQ(astNode.symbolType, model::CppAstNode::SymbolType::FunctionPtr);
    EXPECT_EQ(astNode.astType, model::CppAstNode::AstType::Definition);

    model::CppVariable memberVariable = _db->query_value<model::CppVariable>(
      QCppVariable::name == "memberVariable");
    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::entityHash == memberVariable.entityHash);

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        // case -1: // Access by compiler generated constructors.
        case 44: // Simple access for read.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Read);
          break;

        case 45: // Simple access for write.
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Write);
          break;

        case 7: // Definition
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        default: // Nothing else is allowed.
          EXPECT_TRUE(false);
          break;
      }
  });
}

TEST_F(CppParserTest, Namespace)
{
  _transaction([&, this] {
    model::CppNamespace myNamespace1 = _db->query_value<model::CppNamespace>(
      QCppNamespace::name == "MyNamespace1");
    model::CppAstNode astNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == myNamespace1.entityHash);

    EXPECT_EQ(astNode.symbolType, model::CppAstNode::SymbolType::Namespace);
    EXPECT_EQ(astNode.location.range.start.line, 1);
    EXPECT_EQ(astNode.astType, model::CppAstNode::AstType::Definition);

    model::CppNamespace myNamespace2 = _db->query_value<model::CppNamespace>(
      QCppNamespace::name == "MyNamespace2");
    astNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::entityHash == myNamespace2.entityHash);

    EXPECT_EQ(astNode.symbolType, model::CppAstNode::SymbolType::Namespace);
    EXPECT_EQ(astNode.location.range.start.line, 3);
    EXPECT_EQ(astNode.astType, model::CppAstNode::AstType::Definition);
  });
}
