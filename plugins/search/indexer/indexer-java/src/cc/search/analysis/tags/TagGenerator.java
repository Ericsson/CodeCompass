package cc.search.analysis.tags;

import cc.search.indexer.Context;
import java.io.IOException;

/**
 * Tag generator interface.
 */
public interface TagGenerator extends AutoCloseable {
  /**
   * Generate tags for the given file.
   * 
   * @param tags_ object for storing tags.
   * @param context_ the context.
   * @throws java.io.IOException
   */
  public void generate(Tags tags_, Context context_) throws IOException;
}
