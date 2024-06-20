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
   * Look up and mark indirectly modified files for incremental parsing,
   * based on the semantic information of the parser.
   */
  virtual void markModifiedFiles(){}

  /**
   * Maintains and cleans up the database in preparation of incremental parsing.
   * @return Returns true if the cleanup succeeded, false otherwise.
   */
  virtual bool cleanupDatabase()
  {
    return true;
  }
  /**
   * Method parses a path or a compilation database
   * @return Returns true if the parse succeeded, false otherwise.
   */
  virtual bool parse() = 0;
  /**
   * Returns true in case database indices are required for the parser, due to performance reasons.
   *
   * Should return the same value on each call for the same object.
   * @return Returns true if the database indexing has to be performed before the parser is executed.
   */
  virtual bool isDatabaseIndexRequired() const
  {
    return false;
  }
  
protected:
  ParserContext& _ctx;
};

} // parser
} // cc

#endif	/* CC_PARSER_ABSTRACTPARSER_H */

