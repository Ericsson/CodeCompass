#ifndef CC_MODEL_LSP_TYPES_H
#define CC_MODEL_LSP_TYPES_H

#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

namespace cc
{
namespace service
{
namespace lsp
{

namespace pt = boost::property_tree;

typedef std::string DocumentUri;
typedef std::string Diagram;

/**
 * LSP error codes.
 */
enum class ErrorCode
{
  // Defined by JSON RPC
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603,
  ServerErrorStart = -32099,
  ServerErrorEnd = -32000,
  ServerNotInitialized = -32002,
  UnknownError = -32001,

  // Defined by the protocol
  RequestCancelled = -32800
};

/**
 * Represents an abstract base class for readable LSP structures.
 */
struct Readable
{
  virtual void readNode(const pt::ptree& node) = 0;
};

/**
 * Represents an abstract base class for writeable LSP structures.
 */
struct Writeable
{
  virtual void writeNode(pt::ptree& node) const = 0;

  inline pt::ptree createNode() const
  {
    pt::ptree node;
    writeNode(node);
    return node;
  }
};

/**
 * Represents an LSP response error message.
 */
struct ResponseError : public Writeable
{
  /**
	 * A number indicating the error type that occurred.
	 */
  ErrorCode code;
  /**
	 * A string providing a short description of the error.
	 */
  std::string message;

  void writeNode(pt::ptree& node) const override;
};

/**
 * Text documents are identified using a URI.
 *
 * On the protocol level, URIs are passed as strings.
 */
struct TextDocumentIdentifier : public Readable, public Writeable
{
  /**
	 * The text document's URI.
	 */
  DocumentUri uri;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * Position in a text document expressed as zero-based line and zero-based character offset.
 *
 * A position is between two characters like an ‘insert’ cursor in a editor.
 * Special values like for example -1 to denote the end of a line are not supported.
 */
struct Position : public Readable, public Writeable
{
  /**
	 * Line position in a document (zero-based).
	 */
  int line;
  /**
	 * Character offset on a line in a document (zero-based). Assuming that the line is
	 * represented as a string, the `character` value represents the gap between the
	 * `character` and `character + 1`.
	 *
	 * If the character value is greater than the line length it defaults back to the
	 * line length.
	 */
  int character;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * A range in a text document expressed as (zero-based) start and end positions.
 *
 * A range is comparable to a selection in an editor. Therefore the end position is exclusive.
 * If you want to specify a range that contains a line including the line ending character(s)
 * then use an end position denoting the start of the next line.
 */
struct Range : public Readable, public Writeable
{
  /**
	 * The range's start position.
	 */
  Position start;
  /**
	 * The range's end position.
	 */
  Position end;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * Represents a location inside a resource, such as a line inside a text file.
 */
struct Location final : public Readable, public Writeable
{
  DocumentUri uri;
  Range range;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * A parameter literal used in requests to pass a text document and a position
 * inside that document.
 */
struct TextDocumentPositionParams : public Readable, public Writeable
{
  TextDocumentIdentifier textDocument;
  Position position;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * Represents th context used in requests to retrieve the references.
 */
struct ReferenceContext : public Readable, public Writeable
{
  bool includeDeclaration;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * A parameter literal used in requests to retrieve the references.
 */
struct ReferenceParams : public TextDocumentPositionParams
{
  ReferenceContext context;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * A parameter literal used in requests to retrieve the type of diagrams
 * available for a document and optionally a selected position inside it.
 */
struct DiagramTypeParams : public Readable, public Writeable
{
  TextDocumentIdentifier textDocument;
  boost::optional<Position> position;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * A parameter literal used in requests to retrieve the a specific diagram
 * for a document and optionally a selected position inside it.
 */
struct DiagramParams : DiagramTypeParams
{
  std::string diagramType;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * Represents a possible auto-complete or context menu item.
 */
struct CompletionItem final : public Readable, public Writeable
{
  /**
   * The label of this completion item. By default
   * also the text that is inserted when selecting
   * this completion.
   */
  std::string label;
  /**
	 * The kind of this completion item. Based of the kind
   * an icon is chosen by the editor.
	 */
  boost::optional<int> kind;
  /**
	 * A human-readable string with additional information
	 * about this item, like type or symbol information.
	 */
  boost::optional<std::string> detail;
  /**
	 * A human-readable string that represents a doc-comment.
	 */
  boost::optional<std::string> documentation;
  /**
	 * A data entry field that is preserved on a completion item between
	 * a completion and a completion resolve request.
	 */
  boost::optional<std::string> data;

  void readNode(const pt::ptree& node) override;
  void writeNode(pt::ptree& node) const override;
};

/**
 * Represents a list of completion items.
 */
struct CompletionList : public Writeable
{
  /**
	 * This list it not complete. Further typing should result in recomputing
	 * this list.
	 */
  bool isIncomplete;
  /**
	 * The completion items.
	 */
  std::vector<CompletionItem> items;

  void writeNode(pt::ptree& node) const override;
};

} // lsp
} // service
} // cc

#endif // CC_MODEL_LSP_TYPES_H