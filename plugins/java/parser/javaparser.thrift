namespace cpp cc.parser.java
namespace java cc.parser.java

struct CompileCommand
{
  1: string directory,
  2: string command,
  3: string file
}

struct CmdArgs
{
  1: string directory,
  2: list<string> classpath,
  3: list<string> sourcepath,
  4: string filepath,
  5: string filename,
  6: string bytecodeDir,
  7: list<string> bytecodesPaths
}

exception JavaBeforeParseException
{
    1: string message
}

exception JavaParseException
{
    1: string message
}

service JavaParserService
{
  void parseFile(1: i64 fileId, 2: i32 fileIndex)
    throws (1: JavaBeforeParseException jbe, 2: JavaParseException jpe),
  void setArgs(1: CompileCommand compileCommand),
  CmdArgs getArgs()
}