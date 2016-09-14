package cc.search.analysis;

import java.io.Reader;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.core.LowerCaseFilter;
import org.apache.lucene.util.Version;

/**
 * Analyzer for source text.
 */
public class SourceTextAnalyzer extends Analyzer {
  @Override
  protected TokenStreamComponents createComponents(String f, Reader reader_) {
    final SourceTextTokenizer tokenizer = new SourceTextTokenizer(reader_);
    // Convert tokens to lower-case
    TokenStream tokStream = new LowerCaseFilter(Version.LUCENE_4_9, tokenizer);
    return new TokenStreamComponents(tokenizer, tokStream);
  }

}
