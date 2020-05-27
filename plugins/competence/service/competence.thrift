include "project/common.thrift"

namespace cpp cc.service.competence

struct GitSignature
{
  1:string name,
  2:string email,
  3:i64 time
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

struct UserEmail
{
    1:string email;
    2:string username;
    3:string company;
}

service CompetenceService
{
    string setCompetenceRatio(1:common.FileId fileId, 2:i32 ratio)

    string getDiagram(1:common.FileId fileId, 2:i32 diagramType)

    string getDiagramLegend(1:i32 diagramType)

    list<UserEmail> getUserEmailPairs()

    string setUserData(1:string email, 2:string username, 3:string company)
}