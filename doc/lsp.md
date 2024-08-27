# Supported LSP Requests

## Standard

- [textDocument/declaration](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_declaration)
- [textDocument/definition](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_definition)
- [textDocument/implementation](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_implementation)
- [textDocument/references](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_references)

## CodeCompass Specific Methods

### textDocument/thisCalls

Returns the locations of function calls inside the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/callsOfThis

Returns the locations where the function at the selected position is called.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/callee

Returns the locations of the definitions of the functions called by the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/caller

Returns the locations of the definitions of the functions that call the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/virtualCall

Returns the locations where the function at the selected position may be virtually called.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/functionPointerCall

Returns the locations where the function at the selected position is called through a function pointer.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/parameters

Returns the locations of the parameters of the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/localVariables

Returns the locations of the local variables of the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/overridden

Returns the locations of the functions the function at the selected position overrides.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/overriders

Returns the locations of the functions that override the function at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/read

Returns the locations the variable at the selected position is read.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/write

Returns the locations where the variable at the selected position is written.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/signature

Returns the name of the AST node of the entity at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* string | null

### textDocument/alias

Returns the locations of the type aliases of the type at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/implements

Returns the locations of the types that inherit from the type at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/dataMember

Returns the locations of the data members of the class at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/methods

Returns the locations of the methods of the class at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/friends

Returns the locations of the friends of the class at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/underlyingType

Returns the location of the underlying type of the type alias at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/enumConstants

Returns the locations of the constants of the enumeration at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/expansion

Returns the locations of the expansions of the macro at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/undefinition

Returns the locations of the undefinitions of the macro at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location) | [Location](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#location)[] | null

### textDocument/diagramTypes

Returns the names of diagram types available at the selected position.

*Params:* [TextDocumentPositionParams](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentPositionParams)

*Response:* string[] | null

### textDocument/diagram

Returns a diagram of the selected type based on the selected position in SVG format.

*Params:* diagramParams

*Response:* string | null

### directory/diagram

Returns a diagram of the selected type based on the selected module in SVG format.

*Params:* diagramParams

*Response:* string | null

## CodeCompass Specific Types

### DiagramParams

Parameters needed for diagram requests.
Directory based diagrams do not need the position parameter.
The diagramType should contain a valid diagram type name for the given language plugin.
The `textDocument/diagramTypes` method can be used to access these for position dependant diagrams.

```javascript
interface DiagramParams {
	textDocument: TextDocumentIdentifier;
	position?: Position;
	diagramType: string;
}
```
