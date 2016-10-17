package cc.search.indexer;

import java.util.concurrent.Callable;

/**
 * Async indexer task.
 */
public class IndexerTask implements Callable<Boolean> {
  /**
   * The wrapped indexer.
   */
  private final AbstractIndexer _indexer;
  
  /**
   * @param indexer_ an indexer.
   */
  public IndexerTask(AbstractIndexer indexer_) {
    _indexer = indexer_;
  }
  
  @Override
  public Boolean call() throws Exception {
    return _indexer.index();
  }
}
