#ifndef PARSER_INCLUDEPATH_H
#define PARSER_INCLUDEPATH_H
#include <cstdio>
#include <vector>
#include <sstream>

namespace cc
{
namespace parser
{

class DefaultIncludePathFinder
{
public:
  enum SourceType{UnknownSourceType, CSourceType, CppSourceType};
  static SourceType getTypeOf(std::string sourceFileName_); //not const& on intetion
  static std::vector<std::string> find(const std::string& sourceFileName_, const std::string& prefix_ = "-I");
};

} // parser
} // cc

#endif //PARSER_INCLUDEPATH_H