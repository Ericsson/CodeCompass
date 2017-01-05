/*
 * parseutil.h
 *
 *  Created on: May 7, 2014
 *      Author: ezoltbo
 */

#ifndef PARSER_CXX_PARSEUTIL_H_
#define PARSER_CXX_PARSEUTIL_H_

namespace clang {

class DeclRefExpr;
class ValueDecl;
class QualType;

}

namespace cc
{
namespace parser
{

bool isFunctionPointer(clang::DeclRefExpr* de);

bool isFunctionPointer(clang::ValueDecl* qt);

bool isFunctionPointer(const clang::Type* type);

const clang::Type* getStrippedType(clang::QualType qt);

clang::ValueDecl* getCalleeDecl(clang::CallExpr* ce);

} // parser
} // cc


#endif /* PARSER_CXX_PARSEUTIL_H_ */
