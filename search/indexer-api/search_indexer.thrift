namespace cpp cc.parser.search
namespace java cc.parser.search

/**
 * The name of definitions field for addFieldValues interface method. This
 * field accepts a tag in the following form:
 *    - The location is the tag`s location in the file (required).
 *    - The value is the tag itself (required).
 *    - The context is the tag`s kind (required).
 */
const string FIELD_DEFINITIONS = "fldDefs";

/**
 * The name of "parse status" (virtual) field for addFieldValues method. It
 * accepts only one FieldValue and it is interpreted as follow:
 *    - The value is on of the PSTATUS_* constants (required).
 *    - The other fields should be empty (maybe used in a next release).
 */
const string FIELD_PARSE_STATUS = "fldParseStatus";

/**
 * The file is parsed without errors (a value for FIELD_PARSE_STATUS).
 */
const string PSTATUS_PARSED = "parsed";

/**
 * The file is parsed with errors (a value for FIELD_PARSE_STATUS).
 */
const string PSTATUS_PART_PARSED = "part parsed";

/**
 * The file is not parsed (a value for FIELD_PARSE_STATUS).
 */
const string PSTATUS_NOT_PARSED = "not parsed";


/**
 * Simple location type. The location should be inclusive at the start, but
 * eclusive on the end (as on the Web-UI).
 */
struct Location
{
  /**
   * Start line.
   */
  1: i64 startLine,
  /**
   * Start column.
   */
  2: i64 startColumn,
  /**
   * End line.
   */
  3: i64 endLine,
  /**
   * End column.
   */
  4: i64 endColumn
}

/**
 * A field value is a string and its position.
 */
struct FieldValue
{
 /**
  * The range the source text that will be highlighted.
  * 
  * Must be valid, even if the value is something that is not in the source.
  */
 1:optional Location location,
 /**
  * The searchable value.
  */
 2:string value,
 /**
  * Some additional context.
  */
 3:optional string context
}

/**
 * A field is a map between its name and a vector of string values with their
 * position.
 */
typedef map<string, list<FieldValue>> Fields

/**
 * Interface for search indexer.
 */
service IndexerService
{
  /**
   * Ask the indexer to exit so do not call any other method on this object
   * after stop() method was called.
   */
  oneway void stop(),

  /**
   * Add a file to the index database.
   *
   * @param filePath_ indexable file path.
   * @param fileId_ database id of the file.
   * @param mimeType_ mime type of file.
   */
  oneway void indexFile(
    1:string fileId_,
    2:string filePath_,
    3:string mimeType_),

  /**
   * Adds the given field values to a document. The document will not be
   * created if it does not exists (so it does nothing in this case).
   * 
   * @param fileId_ database id of the file.
   * @param fields_ filed name -> values map.
   */
  oneway void addFieldValues(
    1:string fileId_,
    2:Fields fields_),

  /**
   * (Re)Builds a suggestion index databases. Its a blocking call, so it may
   * take a long time.
   */
  void buildSuggestions(),

  /**
   * Returns the search index statistics:
   *  - Number of documnets in the index
   *  - What suggestion databases has been build
   *  - etc.
   * 
   * @return a key-value map that represents the statistics.
   */
  map<string, string> getStatistics()
}
