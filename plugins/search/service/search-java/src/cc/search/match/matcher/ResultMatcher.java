package cc.search.match.matcher;

import cc.service.search.LineMatch;
import java.io.IOException;
import java.util.List;

/**
 * Interface for matching lines in a search result.
 */
public interface ResultMatcher extends AutoCloseable {
  /**
   * Creates a list of line matches (based on context).
   * 
   * @return matches.
   * @throws IOException 
   */
  public List<LineMatch> match() throws IOException;
}
