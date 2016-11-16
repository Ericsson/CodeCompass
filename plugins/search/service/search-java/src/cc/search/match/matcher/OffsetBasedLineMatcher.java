package cc.search.match.matcher;

import cc.search.analysis.Location;
import cc.search.match.Context;
import cc.service.core.FileRange;
import cc.service.search.LineMatch;
import cc.service.core.Position;
import cc.service.core.Range;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;
import org.apache.lucene.index.Terms;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.highlight.QueryScorer;
import org.apache.lucene.search.highlight.TokenSources;

/**
 * Base class for implementing a line matcher.
 */
class OffsetBasedLineMatcher implements ResultMatcher {
  /**
   * This is from Lucene Highlighter. Limit of character size of a document
   * on a regex/wildcard search.
   */
  private static final int DEFAULT_MAX_CHARS_TO_ANALYZE = 500 * 1024;
  /**
   * Matching context.
   */
  protected final Context _context;
  /**
   * Query scorer.
   */
  private final QueryScorer _scorer;
  /**
   * Current token offset.
   */
  private final OffsetAttribute _offsetAttr;
  /**
   * Token stream.
   */
  private final TokenStream _tokenStream;
  /**
   * Next matching line or null.
   */
  private LineMatch _nextMatch = null;
  
  /**
   * @param context_ matching context.
   * @param query_ query.
   * @param field_ field name.
   * @param tokenStream_ token stream for field.
   * @throws IOException 
   */
  protected OffsetBasedLineMatcher(Context context_, Query query_,String field_,
    TokenStream tokenStream_) throws IOException {
    _context = context_;
    
    // Create a socrer
    _scorer = new QueryScorer(query_, _context.searcher.getIndexReader(),
      field_);
    // This line is for regex and wildcard support.
    _scorer.setMaxDocCharsToAnalyze(DEFAULT_MAX_CHARS_TO_ANALYZE);    

    // Do some init stuff
    tokenStream_.reset();
    TokenStream newStream = _scorer.init(tokenStream_);
    if (newStream != null) {
      tokenStream_ = newStream;
    }
    _tokenStream = tokenStream_;

    // Get the required attributes
    _offsetAttr = _tokenStream.addAttribute(OffsetAttribute.class);
  }

  /**
   * Calculates a line match for the current token in the token stream.
   *
   * @return a line match.
   * @throws IOException
   */
  private LineMatch getCurrentTokenAsLineMatch() {
    final int startOffset = _offsetAttr.startOffset();
    Location loc = _context.lineInfos.offsetToLocation(startOffset, 
      _offsetAttr.endOffset() - startOffset + 1);
    return new LineMatch(new FileRange(_context.fileId, new Range(
      new Position(loc.line, loc.startColumn), new Position(loc.line,
        loc.endColumn))), _context.lineInfos.getLineContent(loc.line));
  }

  /**
   * Reads the token stream for a matching line.
   *
   * @return a matching expression or null on EOF
   * @throws IOException
   */
  private LineMatch matchOne() throws IOException {
    // QueryScorer.startFragment ignores its parameter
    _scorer.startFragment(null);
    LineMatch current = _nextMatch;
    _nextMatch = null;
    while (_tokenStream.incrementToken()) {
      if (_scorer.getTokenScore() <= 0) {
        if (current == null) {
          // Ignore this token
          continue;
        } else {
          return current;
        }
      }
      _nextMatch = getCurrentTokenAsLineMatch();
      if (current == null) {
        current = _nextMatch;
        _nextMatch = null;
      } else if (_nextMatch.range.range.startpos.line ==
        current.range.range.startpos.line) {
        
        // "Multi-token match" merge them
        final Position nextMPos = _nextMatch.range.range.endpos;
        if (nextMPos.line > current.range.range.endpos.line ||
          (nextMPos.line == current.range.range.endpos.line &&
           nextMPos.column > current.range.range.endpos.column)) {
          // The "next" token ends after the current one
          current.range.range.endpos = nextMPos;
        }

        _nextMatch = null;
      } else {
        break;
      }
    }
    return current;
  }

  @Override
  public List<LineMatch> match() throws IOException {
    _tokenStream.reset();
    
    ArrayList<LineMatch> matches = new ArrayList<>(100);
    LineMatch match = matchOne();
    while (match != null) {
      matches.add(match);
      match = matchOne();
    }
    _tokenStream.end();
    return matches;
  }

  @Override
  public void close() throws IOException {
    if (_tokenStream != null) {
      _tokenStream.close();
    }
  }
  
  /**
   * Creates an OffsetBasedLineMatcher for a field with term vector offsets. If
   * the index does not contains term vector for the given field then an 
   * IllegalArgumentException will be thrown.
   * 
   * @param context_ matching context.
   * @param query_ query.
   * @param field_ field name.
   * @throws IOException 
   */
  public static OffsetBasedLineMatcher fromTermVectorField(Context context_,
    Query query_, String field_) throws IOException, IllegalArgumentException {
    final Terms termVector = context_.searcher.getIndexReader().
      getTermVector(context_.documentId, field_);
    if (termVector == null) {
      throw new IllegalArgumentException("Failed to get term vector for field "
        + field_ + "!");
    }
    
    // We need contignous term positions.    
    return new OffsetBasedLineMatcher(context_, query_, field_,
      TokenSources.getTokenStream(termVector, false));
  }
}
