/*
 * utils.h
 *
 *  Created on: Sep 4, 2013
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_CPPSERVICEHELPER_UTILS_H
#define CODECOMPASS_CPPSERVICEHELPER_UTILS_H

#include <vector>
#include <limits>

#include <odb/lazy-ptr.hxx>

#include "language-api/language_types.h"

#include "model/cxx/cppastnode.h"
#include "model/cxx/cpptype.h"
#include "model/cxx/cppfunction.h"
#include "langservicelib/utils.h"

namespace cc
{ 
namespace service
{  
namespace language
{

std::string visibilityToString(model::Visibility visibility);

std::string getSignature(const model::CppFunction& func);

} // language
} // service
} // cc

#endif // CODECOMPASS_CPPSERVICEHELPER_UTILS_H

