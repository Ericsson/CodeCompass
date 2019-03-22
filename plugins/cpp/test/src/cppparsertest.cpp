#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

extern const char* dbConnectionString;

using namespace cc;

using QCppAstNode = odb::query<model::CppAstNode>;
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

    model::CppAstNode simpleFuncAstNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::mangledName == "_Z10singleFuncv");

    EXPECT_EQ(simpleFuncAstNode.astValue, "void singleFunc()");
    EXPECT_EQ(simpleFuncAstNode.mangledName, "_Z10singleFuncv");
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
    model::CppAstNode funcDeclAstNode = _db->query_value<model::CppAstNode>(
      QCppAstNode::mangledName == "_Z8funcDeclv");

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
    RCppAstNode multiFuncAstNode = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName == "_Z13multiFunctionv");

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
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName == "_Z6calleecb");

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
      QCppAstNode::astValue.like("%MyClass(%") &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    model::FileId fileId = astNodes.begin()->location.file.object_id();

    int numDefault = 0, numMove = 0, numCopy = 0;

    for (const model::CppAstNode& n : astNodes)
      if (n.mangledName == "_ZN7MyClassC1Ev") // Default constructor.
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
      else if (n.mangledName == "_ZN7MyClassC1EOS_") // Move constructor.
      {
        EXPECT_TRUE(
          n.location.range.start.line == 20 || // Throwing an exception.
          n.location.range.start.line == 25 || // Returning a value.
          n.location.range.start.line == 35);  // Initializing a field.
        ++numMove;
      }
      else if (n.mangledName == "_ZN7MyClassC1ERKS_") // Copy constructor.
      {
        EXPECT_TRUE(
          n.location.range.start.line == 16 || // Local variable.
          n.location.range.start.line == 17);  // Local variable.
        ++numCopy;
      }
      else
        EXPECT_TRUE(false);

    EXPECT_EQ(numDefault, 7);
    EXPECT_EQ(numMove, 3);
    EXPECT_EQ(numCopy, 2);

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::location.file == fileId &&
      QCppAstNode::symbolType == model::CppAstNode::SymbolType::Function &&
      QCppAstNode::astType == model::CppAstNode::AstType::Usage);

    for (const model::CppAstNode& n : astNodes)
      if (n.mangledName == "_Znwm") // operator new()
        EXPECT_EQ(n.location.range.start.line, 38);
      else if (n.mangledName == "_Znam") // operator new[]()
        EXPECT_EQ(n.location.range.start.line, 42);
      else if (n.mangledName == "_ZdlPv") // operator delete()
        EXPECT_EQ(n.location.range.start.line, 41);
      else if (n.mangledName == "_ZdaPv") // operator delete[]()
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
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName == "Integer");

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
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%MyClass%"));

    for (const model::CppAstNode& n : astNodes)
    {
      if (n.symbolType != model::CppAstNode::SymbolType::Type)
        continue;

      switch (n.location.range.start.line)
      {
        case -1: 
          EXPECT_TRUE(
            // Compiler generated copy and move constructor parameter.
            n.astType == model::CppAstNode::AstType::ParameterTypeLoc ||
            // Compiler generated assignment operator return value.
            // TODO: We lose the information that it is a return type loc.
            //n.astType == model::CppAstNode::AstType::ReturnTypeLoc
            n.astType == model::CppAstNode::AstType::TypeLocation);
          break;

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
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%Enumeration%"));

    for (const model::CppAstNode& n : astNodes)
    {
      if (n.symbolType != model::CppAstNode::SymbolType::Enum)
        continue;

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

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%First%"));

    for (const model::CppAstNode& n : astNodes)
    {
      if (n.symbolType != model::CppAstNode::SymbolType::EnumConstant)
        continue;

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

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%field%"));

    for (const model::CppAstNode& n : astNodes)
    {
      switch (n.location.range.start.line)
      {
        case 32:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          break;

        case 33:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::FunctionPtr);
          break;

        case 35:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Write);
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
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
  });
}

TEST_F(CppParserTest, Variable)
{
  _transaction([&, this] {
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%variable%"));

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case 8:
        case 11:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        case 9:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::Variable);
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Declaration);
          break;

        case 10:
          EXPECT_EQ(n.symbolType, model::CppAstNode::SymbolType::FunctionPtr);
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

    astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::astValue.like("%memberVariable%"));

    for (const model::CppAstNode& n : astNodes)
      switch (n.location.range.start.line)
      {
        case -1: // Access by compiler generated constructors.
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
    RCppAstNode astNodes = _db->query<model::CppAstNode>(
      QCppAstNode::mangledName.like("%MyNamespace%"));

    for (const model::CppAstNode& n : astNodes)
    {
      if (n.symbolType != model::CppAstNode::SymbolType::Namespace)
        continue;

      switch (n.location.range.start.line)
      {
        case 1:
        case 3:
          EXPECT_EQ(n.astType, model::CppAstNode::AstType::Definition);
          break;

        default:
          EXPECT_TRUE(false);
          break;
      }
    }
  });
}
