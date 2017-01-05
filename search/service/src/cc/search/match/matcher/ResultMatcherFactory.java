package cc.search.match.matcher;

import cc.search.match.Context;
import java.io.IOException;

/**
 * Factory interface for ResultMatcher.
 */
public interface ResultMatcherFactory extends AutoCloseable {
  /**
   * Creates a ResultMatcher. This method have to be thread safe.
   * 
   * @param context_ matching context.
   * @return a ResultMatcher or null if not applicable by the context.
   * @throws IOException 
   */
  public ResultMatcher create(Context context_) throws IOException;
}
