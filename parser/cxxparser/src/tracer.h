/*
 * parseutil.h
 *
 *  Created on: May 5, 2014
 *      Author: ezoltbo
 */

#ifndef CXXPARSER_TRACER_H_
#define CXXPARSER_TRACER_H_

#include "util/streamlog.h"

namespace cc
{
namespace parser
{

class Tracer
{
public:
  Tracer(std::string func)
  : funcName(std::move(func))
  {
    std::string indenting(indent, ' ');
    SLog() << "ENTERING: " << indenting << funcName;
    indent += 2;
  }

  ~Tracer()
  {
    indent -= 2;
    std::string indenting(indent, ' ');
    SLog() << "EXITING:  " << indenting << funcName;
  }

private:
  std::string funcName;

  thread_local static unsigned int indent;
};

} // parser
} // cc

#define TRACE cc::parser::Tracer _(__func__);

#endif /* CXXPARSER_TRACER_H_ */
