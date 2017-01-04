include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.search
namespace java cc.service.search

/**
 * Search option flags
 */
enum SearchOptions
{
  /**
   * Do a text search
   */
  SearchInSource    = 0x0001,
  /**
   * Do a symbol search (currently its unimplemented)
   */
  SearchInSymbols   = 0x0002,
  /**
   * Search for file name
   */
  SearchForFileName = 0x0008,
  /**
   * Do a definition search (currently its based on ctags index)
   */
  SearchInDefs      = 0x0010,
  /**
   * Try to find a logging region by a line from a log file.
   */
  FindLogText       = 0x0040
}

/**
 *   The structure to describe a search type
 *   name will be displayed in the gui dropdown menu
 *   searchInWorkspace will be called with 'id' of the selected search type
 */
struct SearchType
{
  1:i32 id,
  2:string name
}

/**
 * Describes a search range.
 */
struct SearchRange
{
  /**
   * Index of the first document in search.
   */
  1:i64 start,
  /**
   * Maximum number of hits.
   */
  2:i64 maxSize
}

/**
 * Describes the filter options for a search.
 */
struct SearchFilter
{
  /**
  * File filter regex.
  */
  1:string  fileFilter,
  /**
   * Directory filter regex.
   */
  2:string  dirFilter
}

/**
 * The ulitmate search options structure.
 */
struct SearchParams
{
  /**
   * Option flags (see SearchOptions enum).
   */
  1:i64 options,
  /**
   * The query.
   */
  2:string query,
  /**
   * Optional search range.
   */
  3: optional SearchRange range,
  /**
   * Optional filter.
   */
  4: optional SearchFilter filter
}

/**
 * Structure for suggestion parameters.
 */
struct SearchSuggestionParams
{
  /**
   * Option flags (see SearchOptions enum).
   */
  1:i64 options,
  /**
   * The maximum size of the result list.
   */
  2:i64 limit = 5,
  /**
   * The raw user input.
   */
  3:string userInput,
  /**
   * A custom tag.
   */
  4:optional string tag;
}

/**
 * The result of a suggest() call.
 */
struct SearchSuggestions
{
  /**
   * The tag that was in the request (SearchSuggestionParams).
   */
  1:optional string tag;
  /**
   * The result texts.
   */
  2:list<string> results;
}

exception SearchException
{
  1:string message
}

exception DatasourceError
{
  1:string message,
  2:i64 errorcode
}

/**
 * Describes a matching line
 */
struct LineMatch
{
  /**
   * The matching range in a file
   */
  1:common.FileRange range,
  /**
   * The matching line
   */
  2:string text
}

/**
 * Describes a search result entry
 */
struct SearchResultEntry
{
  /**
   * List of matching lines in the file's content
   */
  1:list<LineMatch> matchingLines,
  /**
   * File informations
   */
  2:project.FileInfo finfo,
}

/**
 * Describes a search result
 */
struct SearchResult
{
  /**
   * Number of total file matches.
   */
  1:i64 totalFiles,
  /**
   * The results in the actual range: [firstFileIndex, lastFileIndex]
   */
  2:list<SearchResultEntry> results
}

/**
 * Describes a filename search result
 */
struct FileSearchResult
{
  /**
   * Number of total filename matches.
   */
  1:i64 totalFiles,
  /**
   * List of matching files.
   */
  2:list<project.FileInfo> results
}

/**
 * Describes a hit count search result
 */
struct HitCountResult
{
  /**
   * List of matching lines in the file's content
   */
  1:i64 matchingLines,
  /**
   * File informations
   */
  2:project.FileInfo finfo,
}


/**
 * Describes a ranged hit count search result (see SearchService for details)
 */
struct RangedHitCountResult
{
  /**
   * Number of total file matches.
   */
  1:i64                 totalFiles,
  /**
   * The results in the actual range: [firstFileIndex, lastFileIndex]
   */
  2:list<HitCountResult> results,
  /**
   * The original query
   */
  3:string              query
}

/**
 * The search service.
 */
service SearchService
{
  /**
   * Does a text search based on the search database.
   */
  SearchResult search(
    1:SearchParams params_) throws(1:SearchException se),

  /**
   * Does a file search based on SQL database.
   */
  FileSearchResult searchFile(
    1:SearchParams params_)
  throws(
    1:DatasourceError de,
    2:SearchException se),

  /**
   * Returns a list of supported search types
   */
  list<SearchType> getSearchTypes()
    throws (1:common.InvalidId ex),

  /**
   * Kindly asks the Java based implementation to stop and die.
   */
  oneway void pleaseStop(),

  /**
   * Suggests a search text based on the paramaters.
   */
  SearchSuggestions suggest(1:SearchSuggestionParams params_)
    throws (1:SearchException se)
}
