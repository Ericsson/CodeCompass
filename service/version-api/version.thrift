#include "core-api/common.thrift"
#include "core-api/project.thrift"
#include "core-api/result.thrift"

namespace cpp cc.service.version
namespace java cc.service.version

/**
 * Possible Git file modes
 */
enum VersionTreeEntryFileMode {
  GIT_FILEMODE_NEW                = 0x0000,
  GIT_FILEMODE_TREE               = 0x4000,
  GIT_FILEMODE_BLOB               = 0x81A4,
  GIT_FILEMODE_BLOB_EXECUTABLE    = 0x81ED,
  GIT_FILEMODE_LINK               = 0xA000,
  GIT_FILEMODE_COMMIT             = 0xE000,
}

enum VersionDiffDeltaType {
  GIT_DELTA_UNMODIFIED   = 0,
  GIT_DELTA_ADDED        = 1,
  GIT_DELTA_DELETED      = 2,
  GIT_DELTA_MODIFIED     = 3,
  GIT_DELTA_RENAMED      = 4,
  GIT_DELTA_COPIED       = 5,
  GIT_DELTA_IGNORED      = 6, 
  GIT_DELTA_UNTRACKED    = 7,
  GIT_DELTA_TYPECHANGE   = 8,
}

enum VersionObjectType {
  //GIT_OBJ_ANY = -2,       /**< Object can be any of the following */
  //GIT_OBJ_BAD = -1,       /**< Object is invalid. */
  GIT_OBJ__EXT1 = 0,      /**< Reserved for future use. */
  GIT_OBJ_COMMIT = 1,     /**< A commit object. */
  GIT_OBJ_TREE = 2,       /**< A tree (directory listing) object. */
  GIT_OBJ_BLOB = 3,       /**< A file revision object. */
  GIT_OBJ_TAG = 4,        /**< An annotated tag object. */
  GIT_OBJ__EXT2 = 5,      /**< Reserved for future use. */
  GIT_OBJ_OFS_DELTA = 6,  /**< A delta, base is given by an offset. */
  GIT_OBJ_REF_DELTA = 7,  /**< A delta, base is given by object id. */
}



/**
 * Represents a Blob object
 */
struct VersionBlob
{
  /**
   * the repository id in the database
   */
  1:string repoId,

  /**
   * the object id
   */
  2:string oid,

  /**
   * the actual data associated with the id
   */
  3:string data
}

/**
 * Represents a TreeEntry objects
 */
struct VersionTreeEntry
{
  /**
   * the repository id in the database
   */
  1:string repoId,

  /**
   * the object id of the tree
   */
  2:string treeOid,

  /**
   * ordinal number inside the tree node
   */
  3:i32 treeOrdinal

  /**
   * Git file mode
   */
  4:VersionTreeEntryFileMode fileMode

  /**
   * file name int the tree
   */
  5:string fileName

  /**
   * the id of the pointed object
   * - if fileMode is directory, it is a tree object
   * - a blob object otherwise
   */
  6:string pointedOid

}


/**
 * Represents a Commit object
 */
struct VersionCommit
{  
  /**
   * the repository id in the database
   */
  1:string repoId,

  /**
   * the object id of the tree
   */
  2:string oid,

  /**
   * commit message
   */
  3:string message,

  /**
   * first paragraph of the commit message
   */
  4:string summary,

  /**
   * UTC time
   */
  11:i64 time,

  /**
   * time zone
   */
  12:i32 timeOffset,

  /**
   * name of the author
   */
  21:string author,

  /**
   * name of the committer
   */
  22:string committer,

  /**
   * the id of the tree object of the commit root
   */
  31:string treeOid,
  
  /**
   * list of the object ids of the parents of the commit
   */
  32:list<string> parentOids,

}


/**
 * Represents a Tag object
 */
struct VersionTag
{  
  /**
   * the repository id in the database
   */
  1:string repoId,

  /**
   * the object id of the tree
   */
  2:string oid,

  /**
   * commit message
   */
  3:string message,

  /**
   * first paragraph of the commit message
   */
  4:string summary,

  /**
   * first paragraph of the commit message
   */
  5:string name,

  /**
   * UTC time
   */
  11:i64 time,

  /**
   * time zone
   */
  12:i32 timeOffset,

  /**
   * name of the author
   */
  21:string tagger,

  /**
   * the id of the commit object that is tagged
   */
  31:string targetOid,
  
}


/**
 * Represents a old/new file in a delta object
 */
struct VersionDiffDeltaFile
{
    1: string filePath,
    2: i64 fileSize,
    3: string fileOid,
    4: VersionTreeEntryFileMode fileMode,
}
  
/**
 * Represents a delta object in a diff
 */
struct VersionDiffDelta
{
    1: VersionDiffDeltaType type,
    
    10: VersionDiffDeltaFile oldFile,
    11: VersionDiffDeltaFile newFile,

    /* binary seems to be reserved in thrift */
    20: bool isBinary,
    21: bool isNonBinary,
}


/**
 * Represents options to a diff
 */
struct VersionDiffOptions
{
  /**
   * number of context lines
   */
  1: i32 contextLines,
  
  /**
   * if non-empty, only the diff of the filenames matched by one of
   * this list will be returned
   */
  2: list<string> pathspec,

  /**
   * use this commit as starting point instead of parent commit
   */
  10: string fromCommit,
}



/**
 * Represents a repository referenced in the database
 */
struct Repository
{
    1: string id,
    11: string name,
    12: string path,
    14: string pathHash,
    21: bool isHeadDetached,
    22: string head,
}


struct VersionSignature
{
  1: string name,
  2: string email,
  3: i64 time
}

struct VersionBlameHunk
{
  /**
  * the number of lines in this hunk
  */
  1: i32 lines_in_hunk,
  
  /**
  * the OID of the commit where this line was last changed
  */
  2: string final_commit_id;
  
  /**
  * the 1-based line number where this hunk begins, in the final version of
  * the file
  */
  3: i32 final_start_line_number;
  
  4: VersionSignature final_signature;
  
  /**
  * the OID of the commit where this hunk was found. This will usually be
  * the same as final_commit_id, except when
  * GIT_BLAME_TRACK_COPIES_ANY_COMMIT_COPIES has been specified.
  * 
  * TODO implement blame flags
  */
  5: string orig_commit_id;
  
  /**
  * the path to the file where this hunk originated, as of the commit
  * specified by orig_commit_id.
  */
  6: string orig_path;
  
  /**
  * the 1-based line number where this hunk begins in the file named by
  * orig_path in the commit specified by orig_commit_id.
  */
  7: i32 orig_start_line_number;
 
  8: VersionSignature orig_signature;

  /**
  * 1 iff the hunk has been tracked to a boundary commit (the root, or the
  * commit specified in git_blame_options.oldest_commit)
  */
  9: bool boundary;
}


struct RepositoryByProjectPathResult
{
  1: bool isInRepository;
  
  10: string repositoryId;
  11: string commitId;
  12: string repositoryPath;
  
  20: string activeReference;
}

struct ReferenceTopObjectResult
{
  1: string oid;
  2: VersionObjectType type;
}

struct CommitListFilteredResult
{
  1: i32 newOffset;
  2: bool hasRemaining;
  10:list<VersionCommit> result;
}

struct VersionLogEntryDrawinfoEntry
{
  1: i32 from;
  2: i32 to;
  3: string color;
}

struct VersionLogEntryDrawinfo
{
  1: list<VersionLogEntryDrawinfoEntry> l;
  3: i32 dot;
  4: i32 dotStyle;
}

struct VersionLogEntry
{
  10: VersionCommit commit;
  20: VersionLogEntryDrawinfo drawinfo;
}

struct VersionLogResult
{
  10: list<VersionLogEntry> logList;
  20: list<string> drawState;
}

/**
 * The version service.
 */
service VersionService
{
  list<Repository> getRepositoryList(),
  
  /**
   * Retrieves a blob object from the repository
   */
  VersionBlob getBlob(
    1:string repoId_,
    2:string hexOid_),

  /**
   * Retrieves a tree object from the repository
   */
  list<VersionTreeEntry> getTree(
    1:string repoId_,
    2:string hexOid_),

  /**
   * Retrieves a tree object from the repository
   */
  VersionCommit getCommit(
    1:string repoId_,
    2:string hexOid_),

  /**
   * Retrieves a tag object from the repository
   */
  VersionTag getTag(
    1:string repoId_,
    2:string hexOid_),

  /**
   * Retrieves a commit list from the repository starting from a given commit
   * 
   * returns at most count elements. Use count=-1 to return all elements.
   */
  CommitListFilteredResult getCommitListFiltered(
    1:string repoId_,
    2:string hexOid_,
    3:i32 count_,
    4:i32 offset_,
    10:string filter_
  ),

  /**
   * retrieves a list of names for references
   */
  list<string> getReferenceList(
    1:string repoId_,
  ),
    
  /**
   * retrieves a list of names for branches
   */
  list<string> getBrancheList(
    1:string repoId_,
  ),

  /**
   * retrieves a list of names for tags
   */
  list<string> getTagList(
    1:string repoId_,
  ),

  /**
   * retrieves the names for the active reference, empty string if none
   */
  string getActiveReference(
    1:string repoId_,
  ),
    
  /**
   * retrieves the hex object id and type of the head commit of a reference
   */
  ReferenceTopObjectResult getReferenceTopObject(
    1:string repoId_,
    2:string branchName_
  ),
    
  /**
   * Shows the diff of the changes in a commit as a string
   */
  string getCommitDiffAsString(
    1:string repoId_,
    2:string hexOid_,
    3:VersionDiffOptions options_
  ),

  /**
   * Shows the diff of the changes in a commit as a string
   */
  string getCommitDiffAsStringCompact(
    1:string repoId_,
    2:string hexOid_,
    3:VersionDiffOptions options_
  ),

  /**
   * Shows the changes in a commit
   */
  list<VersionDiffDelta> getCommitDiffDeltas(
    1:string repoId_,
    2:string hexOid_,
    3:VersionDiffOptions options_
  ),
  
  /**
   * Retrieves the object id of the blob of a path in a commit
   */
  string getBlobOidByPath(
    1:string repoId_,
    2:string commidOid_,
    3:string path_
  )
  
  /**
   * Retrieves blame for a file in a commit
   */
  list<VersionBlameHunk> getBlameInfo(
    1:string repoId_,
    2:string commidOid_,
    3:string path_,
    10:string localModificationsFileId_
  )
  
  /**
   * Returns if a given project path is in an indexed repository
   */
  RepositoryByProjectPathResult getRepositoryByProjectPath(
    1:string path_
  )
  
  
  /**
   * Returns a revision log with a list of commits ordered by dependency
   * and information requires to draw graph.
   */
  VersionLogResult getFileRevisionLog(
    1:string repoId_,
    2:string branchName_,
    3:string path_,
    4:string continueAtCommit_,
    5:i32 pageSize_,
    10:list<string> continueAtDrawState_
  )
  
}
