/*
 * statistics.h
 *
 *  Created on: Aug 15, 2014
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_MODEL_STATISTICS_H_
#define CODECOMPASS_MODEL_STATISTICS_H_

#include <string>

#include <odb/core.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct Statistics
{
  #pragma db id auto
  unsigned long id;

  std::string group;
  std::string key;
  int         value;
};

} // model
} // cc



#endif /* CODECOMPASS_MODEL_STATISTICS_H_ */
