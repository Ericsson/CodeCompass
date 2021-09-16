namespace cpp cc.parser.java
namespace java cc.parser.java

struct CompileCommand
{
  1: string directory,
  2: string command,
  3: string file
}

service JavaParserService
{
  i32 parseFile(1: CompileCommand jsonPath, 2: i64 fileId)
}