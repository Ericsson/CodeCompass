#ifndef CC_PARSER_ABSTRACTPARSER_H
#define CC_PARSER_ABSTRACTPARSER_H

#include <string>
#include <vector>

#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class AbstractParser
{
public:
  /**
   * Constructor, initialize the parsers
   * @param ctx_ - Parser context options
   */
  AbstractParser(ParserContext& ctx_) : _ctx(ctx_){} 
  
  /**
   * Destructor
   */
  virtual ~AbstractParser(){}

  /**
   * Get dependencies of parsers. 
   * Using these dependencies we will create a linear ordering of the parsers
   * and then we will call the parse() function in the correct order.
   * @return dependent parsers
   */
  virtual std::vector<std::string> getDependentParsers() const = 0;

  /**
   * Maintains and cleans up the database in preparation of
   * incremental parsing.
   * @param dry_ When true, perform a dry-run and only detect the changed files,
   * but do not execute any maintenance actions.
   * @return Return true if the preparse was success, false otherwise.
   */
  virtual bool preparse(bool /* dry_ */ = false)
  {
    return true;
  }

  /**
   * Method parses a path or a compilation database
   * @return Return true if the parse was success, false otherwise
   */
  virtual bool parse() = 0;
  
protected:
  ParserContext& _ctx;
};

} // parser
} // cc

#endif	/* CC_PARSER_ABSTRACTPARSER_H */

