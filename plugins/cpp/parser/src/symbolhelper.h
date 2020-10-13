#ifndef CC_PARSER_SYMBOLHELPER_H
#define CC_PARSER_SYMBOLHELPER_H

#include <string>

#include <clang/AST/Decl.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/Type.h>

#include <model/fileloc.h>

namespace cc
{
namespace parser
{

const clang::Type* getStrippedType(const clang::QualType& qt_);

std::string getMangledName(
  clang::MangleContext* mangleContext_,
  const clang::NamedDecl* nd_,
  const model::FileLoc& fileLoc_ = model::FileLoc());

std::string getMangledName(
  clang::MangleContext* mangleContext_,
  const clang::QualType& qt_,
  const model::FileLoc& fileLoc_ = model::FileLoc());

bool isFunction(const clang::Type* type_);

std::string getSignature(const clang::FunctionDecl* fn_);

}
}

#endif
