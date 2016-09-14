package cc.search.match.matcher;

import cc.search.analysis.query.MatchCollector.MatchInfo;
import cc.search.analysis.query.SimpleMatchCollector;
import cc.search.match.Context;
import cc.search.match.QueryContext;
import cc.service.core.FileRange;
import cc.service.core.LineMatch;
import cc.service.core.Position;
import cc.service.core.Range;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * A result matcher factory for a log queries.
 * 
 * @author Alex Gábor Ispánovics <gabor.alex.ispanovics@ericsson.com>
 */
class LogQueryMatcherFactory implements ResultMatcherFactory {
  /**
   * A result matcher for a log queries.
   */
  private static class LogQueryMatcher implements ResultMatcher {
    /**
     * A collector that contains the matches.
     */
    private final SimpleMatchCollector _collector;
    /**
     * Match context.
     */
    private final Context _context;
    
    /**
     * @param context_ match context.
     * @param collector_  match collector.
     */
    LogQueryMatcher(Context context_, SimpleMatchCollector collector_) {
     _collector = collector_;
     _context = context_;
    }

    @Override
    public List<LineMatch> match() throws IOException {
      final ArrayList<LineMatch> result = new ArrayList<>();
      
      final List<MatchInfo> results = _collector.getResultsInDoc(
        _context.documentId);
      if (results == null) {
        return result;
      }
      
      // Sort matches by score
      Collections.sort(results, new Comparator<MatchInfo>() {
        @Override
        public int compare(MatchInfo i1, MatchInfo i2) {
          return Float.compare(i2.score, i1.score);
        }
      });

      for (final MatchInfo info : results) {
        final int startLine = _context.lineInfos.getLineNumberForOffset(
          info.startOffset);
        final int startColumn = info.startOffset - _context.lineInfos.
          getLineStartOffset(startLine) + 1;
        final int endLine = _context.lineInfos.getLineNumberForOffset(
          info.endOffset);
        final int endColumn = info.endOffset - _context.lineInfos.
          getLineStartOffset(endLine) + 1;
        
        result.add(new LineMatch(new FileRange(_context.fileId, new Range(
          new Position(startLine, startColumn),
          new Position(endLine, endColumn))),
          _context.lineInfos.getLineContent(startLine)));
      }
      
      return result;
    }

    @Override
    public void close() throws Exception {
    }
  }

  @Override
  public ResultMatcher create(Context context_) throws IOException {
    Object data = context_.query.getData(QueryContext.QueryType.Log);
    if (data != null && data instanceof SimpleMatchCollector) {
      return new LogQueryMatcher(context_, (SimpleMatchCollector) data);
    }
    
    return null;
  }

  @Override
  public void close() {
  }
}