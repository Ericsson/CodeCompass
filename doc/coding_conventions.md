Coding conventions
=================

Formatting
----------

- **Length** of a line should not be longer than **80** characters. In the rare
  cases when we need a longer line (e.g. in a preprocessor macro) use backslash
  character to mark a continuation.
- **Tab characters** are **not** welcome in the source. Replace them with
  spaces.
- **Indentation** is 2 characters. Two characters are enough to visually
  emphasize code structure, but do not cause too long lines.
- **Parameter list** is a frequent place, when a line tends to be longer than
  the limit. Break the line and align parameters, as seen in the example below.
  ```cpp
  void aMethodWithManyParameters(
    const std::string& str1_,
    const std::string& str2_,
    const std::int32_t id_);
  ```
- **Namespaces** are not indented.
  ```cpp
  namespace cc
  {
  namespace parser
  {
  /* ... */
  }
  }
  ```
- **Blocks** Related opening and closing brackets should be placed to the same
  column (do not follow Java style). This rule holds for namespaces, classes,
  function blocks and compound statements.
- **Class declarations** should use only one `public`, `protected` and
  `private` part (in this order). The keywords `public`, `protected`,
  `private` are not indented, the member declarations are indented as usual
  (with 2 spaces). Inside a visibility class declare types first.
  ```cpp
  class MyClass
  {
  public:
    int getX();

  protected:
    int _protX;

  private:
    int _privX;
  };
  ```
- **Friend** declarations, if any, should be placed before the public
  visibility items, before the public keyword.

Naming
------

- **File names** should contain lower case ASCII characters. Avoid other
  characters, like dash (-). Header file extension is `.h`, source file
  extension is `.cpp`. Example: `cxxparser.cpp`.
- **Class and Type names** are written in CamelCase. Avoid underscore in class
  or type names. Pointers to major types should be typedef-ed, and should be
  called according the pointed type with a `Ptr` suffix. Example: `Semantic`,
  `FeatureBase`, `SemanticPtr`.
- **Function names** start with lowercase letter, and have a capital letter for
  each new major tag. We do not require different names for static methods, or
  global functions. Example: `getFeature`.
- **Class member names** start with underscore character following a lowercase
  letter, and have a capital letter for each new major tag. Do not use other
  underscores in member names. Example: `_changed`.
- **Function parameter names** start with a lowercase letter, and finish with
  an underscore character, and have a capital letter for each new major tag. Do
  not use other underscores in parameter names. Example: `aParameter_`.
- **Namespace names** are written in lower case. The content of main modules
  are organized in a two-level namespace hierarchy: namespace `cc` contains
  another namespace which describes the main module (e.g. `parser`, `model`,
  `service`).

Headers
-------

- **Interface** header files, i.e. those are intended to be included by other
  libraries should be placed under the _include/submodule_ subdirectory. For
  example the public interface headers of the `core` library is under the
  _include/core_ library. The reason is that other modules should include them
  as `#include <core/semantic.h>` to emphasize that this is an imported
  external header.
- **Implementation** header files, i.e. those are intended only for internal
  use in a library should be placed in the `src` subdirectory and be included
  with quotation marks: `#include "internal.h"`.
  ~~~
  cpp
    |-service
    | |-src
    | | |-cppservice.cpp
    | | |-diagram.h
    | | `-diagram.cpp
    | `-include
    |   `-service
    |     `-cppservice.h
    `-...
  ~~~
- **Include guards** are mandatory for all headers. The guard names are all
  capital letters, and should contain the module hierarchy related to
  namespaces and the file name, separated by underscore characters. For example
  the `cppparser.h` has the following header guard: `CC_PARSER_CPPPARSER_H`.
  ```cpp
  #ifndef CC_PARSER_CPPPARSER_H
  #define CC_PARSER_CPPPARSER_H

  namespace cc
  {
  namespace parser
  {

  class CppParser : public AbstractParser
  {
  /*...*/
  };

  } // parser
  } // cc

  #endif // CC_PARSER_CPPPARSER_H
  ```
- **Order of the inclusion of headers** - either in source files or in other
  header files - should be the following: First include standard C++ headers,
  then Boost headers, then other supporting library headers (ODB, SQLite,
  etc.), then your implementing headers.
  ```cpp
  #include <memory>

  #include <boost/filesystem.hpp>

  #include <model/file.h>
  #include <model/file-odb.hxx>

  #include <util/logutil.h>

  #include "myheader.h"
  ```
- **Never** apply `using namespace` directive in headers.

Examples
--------

## Header file ##
```cpp
#ifndef CC_PARSER_CPPPARSER_H
#define CC_PARSER_CPPPARSER_H

#include <vector>

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <model/buildaction.h>

#include <parser/abstractparser.h>

namespace cc
{
namespace parser
{

class MyParser : public AbstractParser
{
  friend void ifAnyFriendExists();

public:
  MyParser(ParserContext& ctx_);

private:
  void addCompileCommand(
    const clang::tooling::CompileCommand& command_,
    model::BuildActionPtr buildAction_,
    bool error_ = false);

  std::vector<clang::tooling::CompileCommand> _compileCommands;
};

} // parser
} // cc

#endif // CC_PARSER_CPPPARSER_H
```

## Source file ##
```cpp
namespace cc
{
namespace parser
{

MyParser::MyParser(ParserContext& ctx_)
{
  /*...*/
}

void MyParser::addCompileCommand(
  const clang::tooling::CompileCommand& command_,
  model::BuildActionPtr buildAction_,
  bool error_)
{
  /*...*/
}

} // parser
} // cc
```
