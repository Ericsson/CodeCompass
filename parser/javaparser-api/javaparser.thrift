namespace cpp cc.parser
namespace java cc.parser

struct JavaParserArg
{
  1:string database;
  2:string rtJar
  3:string buildId
  4:string sourcepath
  5:list<string> opts
}

enum JavaParsingResult
{
  Success,
  Fail
}

service JavaParserService
{ 
  JavaParsingResult parse(1:JavaParserArg arg),
  oneway void stop()
}