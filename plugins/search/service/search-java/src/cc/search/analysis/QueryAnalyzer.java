package cc.search.analysis;

import cc.search.common.IndexFields;
import java.io.Reader;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.AnalyzerWrapper;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.analysis.util.CharTokenizer;
import org.apache.lucene.util.Version;

/**
 * Analyzer for search queries.
 */
public class QueryAnalyzer extends AnalyzerWrapper {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger(QueryAnalyzer.class.
    getName());
  /**
   * A simple analyzer for queries.
   */
  private static class SimpleAnalyzer extends Analyzer {
    /**
     * Simple tokenizer implementation. Its a WhitespaceTokenizer and a 
     * LowerCaseTokenizer combinated.
     */
    private static class SimpleTokenizer extends CharTokenizer {
      /**
       * @param reader_ a reader.
       */
      public SimpleTokenizer(Reader reader_) {
        super(Version.LUCENE_4_9, reader_);
      }

      @Override
      protected boolean isTokenChar(int char_) {
        return !Character.isWhitespace(char_);
      }

      @Override
      protected int normalize(int char_) {
        return Character.toLowerCase(char_);
      }
    }
    
    @Override
    protected TokenStreamComponents createComponents(String f, Reader reader_) {
     return new TokenStreamComponents(new SimpleTokenizer(reader_));
    }
    
  }
  /**
   * Text analyzer.
   */
  private final Analyzer _textAnalyzer = new SourceTextAnalyzer();
  /**
   * Fallback analyzer.
   */
  private final Analyzer _fbAnalyzer = new StandardAnalyzer(Version.LUCENE_4_9);
  /**
   * Analyzer for several fields.
   */
  private final Analyzer _simpleAnalyzer = new SimpleAnalyzer();
  
  public QueryAnalyzer() {
    super(PER_FIELD_REUSE_STRATEGY);
  }

  @Override
  protected Analyzer getWrappedAnalyzer(String field_) {
    if (IndexFields.isTagKindFieldName(field_)) {
      return _simpleAnalyzer;
    }
    
    switch (field_) {
      case IndexFields.contentField:
        return _textAnalyzer;
      case IndexFields.mimeTypeField:
      case IndexFields.definitionsField:
        return _simpleAnalyzer; 
      default:
        _log.log(Level.WARNING, "Unknown field ({0}) in a query. Falling back" +
          " to StandardAlayzer!", field_);
        return _fbAnalyzer;
    }
  }

  @Override
  public void close() {
    super.close();
    
    _fbAnalyzer.close();
    _simpleAnalyzer.close();
  }
}
