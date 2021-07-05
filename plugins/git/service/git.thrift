namespace cpp cc.service.git

enum GitObjectType
{
  Reserved1 = 0,  /**< Reserved for future use in Libgit2. */
  Commit = 1,     /**< A commit object. */
  Tree = 2,       /**< A tree (directory listing) object. */
  Blob = 3,       /**< A file revision object. */
  Tag = 4,        /**< An annotated tag object. */
  Reserved2 = 5,  /**< Reserved for future use in Libgit2. */
  OffetDelta = 6, /**< A delta, base is given by an offset. */
  RefDelta = 7,   /**< A delta, base is given by object id. */
}

struct GitSignature
{
  1:string name,
  2:string email,
  3:i64 time
}

struct GitCommit
{
  1:string repoId,           /**< Repository ID. */
  2:string oid,              /**< Unique identity of the commit. */
  3:string message,          /**< Full message of the commit. */
  4:string summary,          /**< Short summary of the git commit message. */
  5:i64 time,                /**< UTC time. */
  6:GitSignature author,     /**< Author signature. */
  7:GitSignature committer,  /**< Committer signature. */
  8:string treeOid,          /**< Id of the tree pointed to by a commit. */
  9:list<string> parentOids, /**< Specified parent of the commit. */
}

struct GitTag
{
  1:string repoId,    /**< Repository ID. */
  2:string oid,       /**< Unique identity of the tag. */
  3:string message,   /**< Full message of the tag. */
  4:string name,      /**< The name of the tag. */
  5:string tagger,    /**< The tagger (author) of the tag. */
  6:string targetOid, /**< OID of the tagged object of the tag. */
}

struct GitDiffOptions
{
  1:i32 contextLines,      /**< Number of context lines. */
  2:list<string> pathspec, /**< If non-empty, only the diff of the filenames
                                matched by one of this list will be returned.
                           */
  3:string fromCommit      /**< Use this commit as starting point instead of
                                parent commit. */
}

struct GitRepository
{
  1:string id,           /**< Unique ID of the repository. */
  2:string path,         /**< Path of the repository. */
  3:bool isHeadDetached, /**< True if a repository's HEAD is detached. */
  4:string head,         /**< Full name of a head reference. */
  5:string name          /**< Name of the repository. */
}

struct ReferenceTopObjectResult
{
  1:string oid;
  2:GitObjectType type;
}

struct CommitListFilteredResult
{
  1:i32 newOffset;
  2:bool hasRemaining;
  3:list<GitCommit> result;
}

struct RepositoryByProjectPathResult
{
  1:bool isInRepository; /**< True if path is in repository. */
  2:string repoId;
  3:string repoPath;
  4:string commitId;
  5:string activeReference;
}

struct GitBlameHunk
{
  1:i32 linesInHunk,             /**< The number of lines in this hunk. */
  2:string finalCommitId,        /**< The OID of the commit where this line was
                                      last changed. */
  3:string finalCommitMessage,   /**< Commit message of the commit specified by
                                      finalCommitId. */
  4:i32 finalStartLineNumber;    /**< The 1-based line number where this hunk
                                      begins, in the final version of the file.
                                 */
  5:GitSignature finalSignature; /**< Signature of the commit who this line
                                      last changed. */
  6:string origCommitId;         /**< The OID of the commit where this hunk was
                                      found. This will usually be the same as
                                      final_commit_id, except when
                                      GIT_BLAME_TRACK_COPIES_ANY_COMMIT_COPIES
                                      has been specified.
                                      TODO: implement blame flags. */
  7:string origPath;             /**< The path to the file where this hunk
                                      originated, as of the commit specified by
                                      origCommitId. */
  8:i32 origStartLineNumber;     /**< The 1-based line number where this hunk
                                      begins in the file named by orig_path in
                                      the commit specified by origCommitId. */
  9:GitSignature origSignature;  /**< Signature of the commit who thisline last
                                      changed. */
  10:bool boundary;              /**< True if the hunk has been tracked to a
                                      boundary commit (the root, or the commit
                                      specified in
                                      git_blame_options.oldest_commit). */
}

service GitService
{
  /**
   * This function returns the available repositories.
   */
  list<GitRepository> getRepositoryList()

  /**
   * Returns if a given project path is in an indexed repository.
   */
  RepositoryByProjectPathResult getRepositoryByProjectPath(
    1:string path_)

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
  list<string> getBranchList(
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

  /**
   * Retrieves the object id of the blob of a path in a commit.
   */
  string getBlobOidByPath(
    1:string repoId_,
    2:string hexOid_,
    3:string path_)

  /**
   * Retrieves a blob content.
   */
  string getBlobContent(
    1:string repoId_,
    2:string hexOid_),

  /**
   * Retrieves blame for a file in a commit.
   */
  list<GitBlameHunk> getBlameInfo(
    1:string repoId_,
    2:string hexOid_,
    3:string path_,
    4:string localModificationsFileId_),

   /**
   * Check whether there is at least one repository
   * in the workspace directory.
   */
    bool isEnabled()
}
