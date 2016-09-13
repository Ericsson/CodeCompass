#ifndef CC_PARSER_SYMBOLHELPER_H
#define CC_PARSER_SYMBOLHELPER_H

#include <string>

#include <clang/AST/Mangle.h>

namespace cc
{
namespace parser
{

class SymbolHelper
{
public:
  SymbolHelper(clang::ASTContext& astContext_);

  std::string getMangledName(
    const clang::NamedDecl* nd_,
    const model::FileLoc& fileLoc_ = model::FileLoc()) const;

  std::string getMangledName(
    const clang::QualType& qt_,
    const model::FileLoc& fileLoc_ = model::FileLoc()) const;

private:
  clang::MangleContext* _mangleContext;
};

}
}

#endif
