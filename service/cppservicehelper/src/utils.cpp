/*
 * utils.h
 *
 *  Created on: Sep 4, 2013
 *      Author: ezoltbo
 */

#include "utils.h"

#include <string>

#include "util/streamlog.h"

#include "model/file.h"
#include "model/file-odb.hxx"
#include "model/cxx/cppastnode-odb.hxx"
#include "model/cxx/cpptype-odb.hxx"
#include "model/cxx/cppfunction-odb.hxx"

namespace cc
{ 
namespace service
{  
namespace language
{

std::string visibilityToString(model::Visibility visibility)
{
  switch (visibility)
  {
    case model::Public:
      return "public";
    case model::Protected:
      return "protected";
    case model::Private:
      return "private";
  }

  return "Unknown";
}

std::string getSignature(const model::CppFunction& func)
{
  int firstPar = func.name.find_first_of('(');

  int funcNameStart = func.name.find_last_of(' ', firstPar) + 1;

  return func.name.substr(funcNameStart);
}

} // language
} // service
} // cc

