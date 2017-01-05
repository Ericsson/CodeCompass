namespace cpp cc.parser
namespace java cc.parser

struct TFileLoc
{
  1 : i32     lineStart,
  2 : i32     offset,
  3 : i32     lineEnd,
  4 : i32     endOffset,
  5 : string  file
}

struct TPythonAstNode
{
  1 : string   id,
  2 : string   name,
  3 : string   abv_qname,
  4 : i32      astType,
  5 : TFileLoc location,
  6 : string   baseBinding,
  7 : string   containerBinding,
  8 : bool     globalWrite
}

struct TPythonBinding
{
  1 : string   id,
  2 : string   name,
  3 : string   formattedQname,
  4 : string   mangledName,
  5 : i32      kind,
  6 : string   type,
  7 : TFileLoc location,
  8 : string   documentation
}

struct TPythonReference
{
  1 : string node,
  2 : string binding
}

struct TPythonUnknown
{
  1 : string   id,
  2 : string   target,
  3 : string   name,
  4 : i32      kind
}

struct TPythonClassDef
{
  1 : string target,
  2 : string constructor
}

struct TPythonFunctionDef
{
  1 : string id,
  2 : string memberOf,
  3 : i32 minParamNum,
  4 : i32 maxParamNum
}

struct TPythonFunctionCall
{
  1 : string id,
  2 : i32 argNum
}

struct TPythonFunctionParam
{
  1 : string id,
  2 : string target
}

struct TPythonDecorator
{
  1 : string id,
  2 : string target,
  3 : string value
}

struct TPythonInheritance
{
  1 : string target,
  2 : string base
  3 : i32    kind
}

struct TPythonAttribute
{
  1 : string attribute,
  2 : string target
}

struct TPythonVariable
{
  1 : string id,
  2 : string target,
  3 : bool   isGlobal
}

enum TPythonVariableRefKind {
  Definition,   ///< This a definition. A definition is also a write.
  Read,         ///< Read reference.
  Write         ///< Write reference (the variable is already defined).
}

struct TPythonVariableRef
{
  1 : string nodeId,
  2 : string mangledName,
  3 : TPythonVariableRefKind kind
}


service PythonPersisterService
{
  void stop(),

  void addNode(1 : TPythonAstNode node),
  void addBinding(1 : TPythonBinding binding),
  void addReference(1 : TPythonReference reference),
  void addUnknown(1 : TPythonUnknown unknown),
  void addClassDef(1 : TPythonClassDef classDef),
  void addFunctionDef(1 : TPythonFunctionDef functionDef),
  void addFunctionCall(1 : TPythonFunctionCall functionCall),
  void addFunctionParam(1 : TPythonFunctionParam functionParam),
  void addDecorator(1 : TPythonDecorator decorator),
  void addInheritance(1 : TPythonInheritance inheritance),
  void addAttribute(1 : TPythonAttribute attribute),
  void addVariable(1 : TPythonVariable variable),
  void addVariableRef(1 : TPythonVariableRef variable),

  void failedToParse(1:string filePath)
}
