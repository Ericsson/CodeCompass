namespace cpp cc.service.git

enum GitObjectType
{
  GIT_OBJ__EXT1 = 0,      /**< Reserved for future use. */
  GIT_OBJ_COMMIT = 1,     /**< A commit object. */
  GIT_OBJ_TREE = 2,       /**< A tree (directory listing) object. */
  GIT_OBJ_BLOB = 3,       /**< A file revision object. */
  GIT_OBJ_TAG = 4,        /**< An annotated tag object. */
  GIT_OBJ__EXT2 = 5,      /**< Reserved for future use. */
  GIT_OBJ_OFS_DELTA = 6,  /**< A delta, base is given by an offset. */
  GIT_OBJ_REF_DELTA = 7,  /**< A delta, base is given by object id. */
}

struct GitCommit
{
  /**
   * Repository id.
   */
  1:string repoId,

  /**
   * Unique identity of the commit.
   */
  2:string oid,

  /**
   * Full message of the commit.
   */
  3:string message,

  /**
   * Short summary of the git commit message.
   */
  4:string summary,

  /**
   * UTC time.
   */
  5:i64 time,

  /**
   * Commit timezone offset.
   */
  6:i32 timeOffset,

  /**
   * Commit author
   */
  7:string author,

  /**
   * Committer of the commit.
   */
  8:string committer,

  /**
   * Id of the tree pointed to by a commit
   */
  9:string treeOid,

  /**
   * Specified parent of the commit.
   */
  10:list<string> parentOids,
}

struct GitTag
{
  /**
   * Repository id.
   */
  1: string repoId,

  /**
   * Unique identity of the tag.
   */
  2:string oid,

  /**
   * Full message of the tag.
   */
  3:string message,

  /**
   * The name of the tag.
   */
  4: string name,

  /**
   * The tagger (author) of the tag.
   */
  5: string tagger,

  /**
   * OID of the tagged object of the tag.
   */
  6: string targetOid,
}

struct GitDiffOptions
{
  /**
   * Number of context lines.
   */
  1: i32 contextLines,

  /**
   * If non-empty, only the diff of the filenames matched by one of
   * this list will be returned.
   */
  2: list<string> pathspec,

  /**
   * Use this commit as starting point instead of parent commit.
   */
  3: string fromCommit
}

struct GitRepository
{
  /**
   * Unique id of the repository
   */
  1: string id,

  /**
   * Path of the repository.
   */
  2: string path,

  /**
   * True if a repository's HEAD is detached.
   */
  3: bool isHeadDetached,

  /**
   * Full name of a head reference.
   */
  4: string head,
}

struct ReferenceTopObjectResult
{
  1: string oid;
  2: GitObjectType type;
}

struct CommitListFilteredResult
{
  1: i32 newOffset;
  2: bool hasRemaining;
  3: list<GitCommit> result;
}

service GitService
{
  /**
   * This function returns the available repositories.
   */
  list<GitRepository> getRepositoryList()

  /**
   * Returns a commit object from the repository.
   */
  GitCommit getCommit(
    1:string repoId_,
    2:string hexOid_)

  /**
   * Returns a tag object from the repository.
   */
  GitTag getTag(
    1:string repoId_,
    2:string hexOid_)

  /**
   * Retrieves a commit list from the repository starting from a given commit
   * returns at most count elements. Use count=-1 to return all elements.
   */
  CommitListFilteredResult getCommitListFiltered(
    1:string repoId_,
    2:string hexOid_,
    3:i32 count_,
    4:i32 offset_,
    10:string filter_)

  /**
   * Returns a list with all the references that can be found in the repository.
   */
  list<string> getReferenceList(
    1:string repoId_)

  /**
   * Return a list with all the branches that can be found in the repository.
   */
  list<string> getBrancheList(
    1:string repoId_)

  /**
   * Return a list with all the tags that can be found in the repository.
   */
  list<string> getTagList(
    1:string repoId_)

  /**
   * Retrieves the hex object id and type of the head commit of a reference.
   */
  ReferenceTopObjectResult getReferenceTopObject(
    1:string repoId_,
    2:string branchName_)

  /**
   * Create a diff with the difference between two tree objects.
   */
  string getCommitDiffAsString(
    1:string repoId_,
    2:string hexOid_,
    3:GitDiffOptions options_,
    4:bool isCompact)
}
