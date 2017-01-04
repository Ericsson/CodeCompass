package cc.search.analysis.query;

/**
 * Collector interface for query matches.
 * 
 * @author Alex Gábor Ispánovics <gabor.alex.ispanovics@ericsson.com>
 */
public interface MatchCollector {
  /**
   * Informations (position and score) about a match.
   */
  public static class MatchInfo {
    /**
     * The score of this match.
     */
    public final float score;
    /**
     * Start offset for the match.
     */
    public final int startOffset;
    /**
     * End offset for the match.
     */
    public final int endOffset;
    /**
     * @param score_ @see MatchInfo#score
     * @param startOffset_ @see MatchInfo#startOffset
     * @param endOffset_  @see MatchInfo#endOffset
     */
    MatchInfo(float score_, int startOffset_, int endOffset_) {
      score = score_;
      startOffset = startOffset_;
      endOffset = endOffset_;
    }
  }
  /**
   * Collect a match.
   * 
   * @param docId_ high level document id (base offset already added to it).
   * @param info_ match informations.
   */
  public void collectMatch(int docId_, MatchInfo info_);
}
