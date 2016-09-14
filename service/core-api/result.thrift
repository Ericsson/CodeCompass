include "common.thrift"
include "project.thrift"

namespace cpp cc.service.core
namespace java cc.service.core

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
  2:project.BasicFileInfo finfo,
}

/**
 * Describes a search result
 */
struct SearchResult
{
  /**
   * The index of the first file in the results list.
   */
  1:i64 firstFileIndex,
  /**
   * Number of total file matches.
   */
  3:i64 totalFiles,
  /**
   * The results in the actual range: [firstFileIndex, lastFileIndex]
   */
  4:list<SearchResultEntry> results
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
  2:list<project.BasicFileInfo> results
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
  2:project.BasicFileInfo finfo,
}


/**
 * Describes a ranged hit count search result (see SearchService for details)
 */
struct RangedHitCountResult
{
  /**
   * The index of the first file in the results list.
   */
  1:i64                 firstFileIndex,
  /**
   * Number of total file matches.
   */
  3:i64                 totalFiles,
  /**
   * The results in the actual range: [firstFileIndex, lastFileIndex]
   */
  4:list<HitCountResult> results,
  /**
   * The original query
   */
  5:string              query
}
