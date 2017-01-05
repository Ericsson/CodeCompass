#ifndef PARSER_JAVAPARSERAPI_JAVAPARSERPROCESS_H
#define PARSER_JAVAPARSERAPI_JAVAPARSERPROCESS_H

#include <memory>
#include <string>
#include <vector>

#include <util/thriftprocess.h>
#include <javaparser-api/JavaParserService.h>

namespace cc
{
namespace parser
{

/**
 * Indexer process. Currently it's a Java application.
 */
class JavaParserProcess :
  public util::ThriftProcess,
  public parser::JavaParserServiceIf
{
public:
  JavaParserProcess();
  
  ~JavaParserProcess();
  
public:
  virtual JavaParsingResult::type parse(const JavaParserArg& arg_) override;
  virtual void stop() override;
  
private:
  std::unique_ptr<parser::JavaParserServiceIf> _javaParser;
};

} // parser
} // cc

#endif // PARSER_JAVAPARSERAPI_JAVAPARSERPROCESS_H
