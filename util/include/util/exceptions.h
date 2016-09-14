/*
 * exceptions.h
 *
 *  Created on: Jul 29, 2013
 *      Author: ezoltbo
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <exception>

namespace cc 
{
namespace util 
{

struct ItemNotFound : std::runtime_error
{
  ItemNotFound() : std::runtime_error("") {}

  explicit ItemNotFound(const std::string& msg) : std::runtime_error(msg) { }
};

} // util
} // cc


#endif /* EXCEPTIONS_H_ */
