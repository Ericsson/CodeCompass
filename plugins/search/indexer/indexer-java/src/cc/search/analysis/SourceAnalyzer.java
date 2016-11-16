package cc.search.analysis;

import cc.search.common.IndexFields;
import java.io.Reader;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.AnalyzerWrapper;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.core.LowerCaseFilter;
import org.apache.lucene.analysis.path.PathHierarchyTokenizer;
import org.apache.lucene.analysis.util.CharTokenizer;
import org.apache.lucene.util.Version;

/**
 * The analyzer for indexing source files.
 */
public final class SourceAnalyzer extends AnalyzerWrapper {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger(SourceAnalyzer.class
    .getName());
  /**
   * Analyzer for a full path.
   */
  private static class FullPathAnalyzer extends Analyzer {
    @Override
    protected TokenStreamComponents createComponents(String f, Reader reader_) {
      final Tokenizer tokenizer = new PathHierarchyTokenizer(reader_);
      // Convert tokens to lower-case
      TokenStream tokStream = new LowerCaseFilter(Version.LUCENE_4_9, tokenizer);

      return new TokenStreamComponents(tokenizer, tokStream);
    }
  }
  /**
   * Analyzer for tag kinds filed.
   */
  private static class TagKindAnalyzer extends Analyzer {
    @Override
    protected TokenStreamComponents createComponents(String f, Reader reader_) {
      final Tokenizer tokenizer = new CharTokenizer(Version.LUCENE_4_9, reader_){
        @Override
        protected boolean isTokenChar(int char_) {
          return char_ != '\n';
        }
      };
      // Convert token to lower-case
      TokenStream tokStream = new LowerCaseFilter(Version.LUCENE_4_9, tokenizer);
      
      return new TokenStreamComponents(tokenizer, tokStream);
    }  
  }
  /**
   * Analyzer for source text.
   */
  private static final SourceTextAnalyzer _sourceAnal = 
    new SourceTextAnalyzer();
  /**
   * Analyzer for a full path.
   */
  private static final FullPathAnalyzer _fullPathAnal =
    new FullPathAnalyzer();
  /**
   * Analyzer for tag kinds filed.
   */
  private static final TagKindAnalyzer _tagKindAnal =
    new TagKindAnalyzer();
  /**
   * Creates a new SourceAnalyzer with PER_FIELD_REUSE_STRATEGY.
   */
  public SourceAnalyzer() {
    super(PER_FIELD_REUSE_STRATEGY);
  }
  
  @Override
  protected Analyzer getWrappedAnalyzer(String field_) {    
    switch (field_) {
      case IndexFields.contentField:
        return _sourceAnal;
      case IndexFields.filePathField:
        return _fullPathAnal;
      case IndexFields.tagKindsField:
        return _tagKindAnal;
      default:
        _log.log(Level.SEVERE, "No Analyzer for {0}", field_);
        throw new AssertionError(field_);
    }
  }
  
  @Override
  public int getOffsetGap(String fieldName_) {
    if (IndexFields.isTagKindFieldName(fieldName_) ||
      fieldName_.equals(IndexFields.definitionsField)) {
      return 1;
    } else {
      return super.getOffsetGap(fieldName_);
    }
  }

  @Override
  public int getPositionIncrementGap(String fieldName_) {
    if (IndexFields.isTagKindFieldName(fieldName_) ||
      fieldName_.equals(IndexFields.definitionsField)) {
      return 1;
    } else {
      return super.getPositionIncrementGap(fieldName_);
    }
  }

  @Override
  public void close() {
    super.close();
    
    _sourceAnal.close();
    _fullPathAnal.close();
    _tagKindAnal.close();
  }
}
