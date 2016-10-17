#ifndef CC_PARSER_INDEXERPROCESS_H
#define CC_PARSER_INDEXERPROCESS_H

#include <unordered_map>
#include <string>
#include <vector>

#include <util/pipedprocess.h>
#include <IndexerService.h>

namespace cc
{
namespace parser
{

/**
 * Indexer process. Currently it's a Java application.
 */
class IndexerProcess :
  public util::PipedProcess,
  public search::IndexerServiceIf
{
public:
  /**
   * Database open mode.
   */
  enum class OpenMode
  {
    /**
     * Create / overwrite a new index database.
     */
    Create,
    /**
     * Replace documents with their new versions and keep the unchanged ones.
     */
    ReplaceExisting,
    /**
     * Merge new and old documents.
     */
    Merge
  };
  
  /**
   * Database locking modes.
   */
  enum class LockMode {
    /**
     * Use native locks. This is a stable and recommend mode for locking file
     * but on NFS this isn't works.
     */
    Native,
    /**
     * This locking mechanism is based on a little buggy Java feature but in
     * the most cases it's works (see SimpleFSLockFactory in Lucene doc). This
     * locking mechanism works also on NFS.
     */
    Simple
  };

public:
  /**
   * Opens the indexer process.
   *
   * FIXME: it would be nice if we could detect the filesystem type and only
   * use Simple lock mode on NFS.
   *
   * @param indexDatabase_ path to index database.
   * @param openMode_ database open mode.
   * @param lockMode_ Lucene database lock mode.
   */
  IndexerProcess(
    const std::string& indexDatabase_,
    OpenMode openMode_,
    LockMode lockMode_ = LockMode::Simple);
  
  /**
   * Closes the I/O pipe so the child process will exit if it finished. Also
   * waits for the process to exit.
   */
  ~IndexerProcess();
  
public:
  virtual void stop() override;
  
  virtual void indexFile(
    const std::string& fileId_,
    const std::string& filePath_,
    const std::string& mimeType_) override;
  
  virtual void addFieldValues(
    const std::string& fileId_,
    const search::Fields& fields_) override;

  virtual void buildSuggestions() override;

  virtual void getStatistics(
    std::map<std::string, std::string>& stat_) override;
  
private:
  /**
   * Second pipe for thrift.
   */
  int _pipeFd2[2];
  /**
   * Indexer interface for IPC communication.
   */
  std::unique_ptr<search::IndexerServiceIf> _indexer;
};

} // parser
} // cc

#endif // CC_PARSER_INDEXERPROCESS_H
