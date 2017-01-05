// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_ASTTYPE_H
#define MODEL_ASTTYPE_H

#include <string>

// The enums should be updated in:
//   * model/src/astnode.cpp
//   * parser/javaparser-java/src/parser/JavaAstNode.java

// TODO: this file is deprecated now, it is not used by CppAstNode anymore

namespace cc
{
namespace model
{

enum class SymbolType
{
  Variable,
  Function,
  FunctionPtr,
  Type,
  Typedef,
  Macro,
  Enum,
  EnumConstant,
  Import,
  File = 500,
  Other = 1000 // TODO: should be the first (for value zero)
};

enum class AstType
{
  Statement,
  TypeLocation,
  Declaration,
  Definition,
  UnDefinition, // for Macros
  Usage,
  Read,
  Write,
  VirtualCall,
  ParameterTypeLoc,
  ReturnTypeLoc,
  FieldTypeLoc,
  GlobalTypeLoc,
  LocalTypeLoc,
  Other = 1000 // TODO: should be the first (for value zero)
};

std::string symbolTypeToString(SymbolType symboltype);

std::string astTypeToString(AstType asttype);

} // model
} // cc

#endif // MODEL_ASTTYPE_H

