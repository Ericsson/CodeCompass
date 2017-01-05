// $Id$
// Created by Aron Barath, 2013

#include <string>

#include <model/asttype.h>

namespace cc
{
namespace model
{

std::string symbolTypeToString(SymbolType symboltype)
{
  switch(symboltype)
  {
    case SymbolType::Other:        return "Other";
    case SymbolType::Variable:     return "Variable";
    case SymbolType::Function:     return "Function";
    case SymbolType::FunctionPtr:  return "Function Pointer";
    case SymbolType::Type:         return "Type";
    case SymbolType::Typedef:      return "Typedef";
    case SymbolType::Macro:        return "Macro";
    case SymbolType::Enum:         return "Enum";
    case SymbolType::EnumConstant: return "Enum Constant";
    case SymbolType::Import:       return "Import";
    case SymbolType::File:         return "File";

    default:                       return "?unknown?";
  }
}

std::string astTypeToString(AstType asttype)
{
  switch(asttype)
  {
    case AstType::Other:            return "Other";
    case AstType::Statement:        return "Statement";
    case AstType::TypeLocation:     return "TypeLocation";
    case AstType::Declaration:      return "Declaration";
    case AstType::UnDefinition:     return "UnDefinition";
    case AstType::Definition:       return "Definition";
    case AstType::Usage:            return "Usage";
    case AstType::Read:             return "Read";
    case AstType::Write:            return "Write";
    case AstType::VirtualCall:      return "VirtualCall";
    case AstType::ParameterTypeLoc: return "ParameterTypeLoc";
    case AstType::ReturnTypeLoc:    return "ReturnTypeLoc";
    case AstType::GlobalTypeLoc:    return "GlobalTypeLoc";
    case AstType::LocalTypeLoc:     return "LocalTypeLoc";
    case AstType::FieldTypeLoc:     return "FieldTypeLoc";

    default:                        return "?unknown?";
  }
}

} // model
} // cc

