#ifndef CC_PARSER_EFFERENT_H
#define CC_PARSER_EFFERENT_H

#include "parser/parsercontext.h"

namespace cc
{
namespace parser
{

class EfferentCoupling
{
public:
  EfferentCoupling(
    ParserContext& ctx_,
    std::vector<std::string> inputPaths_);
  void efferentTypeLevel();

private:
  ParserContext _ctx;

  std::vector<std::string> _inputPaths;
};

} // parser
} // cc

#endif // CC_PARSER_EFFERENT_H
