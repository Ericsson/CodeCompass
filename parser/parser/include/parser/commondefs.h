#ifndef PARSER_COMMONDEFS_H
#define PARSER_COMMONDEFS_H

#include <model/project.h>
#include <map>

namespace cc
{
namespace parser
{

struct ParseProps
{
  model::ProjectPtr project;
  std::map<std::string, std::string> options;
};

enum ParseResult
{
  PARSE_SUCCESS  = 0,
  PARSE_FAIL     = 1,
  PARSE_DEFERRED = 2,
};

}
}

#endif
