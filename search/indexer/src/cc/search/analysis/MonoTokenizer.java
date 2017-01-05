package cc.search.analysis;

import java.io.Reader;
import org.apache.lucene.analysis.util.CharTokenizer;
import org.apache.lucene.util.AttributeFactory;
import org.apache.lucene.util.Version;

/**
 * Creates one token from the whole text.
 */
final class MonoTokenizer extends CharTokenizer {
  /**
   * @param reader_ input.
   */
  public MonoTokenizer(Reader reader_) {
    super(Version.LUCENE_4_9, reader_);
  }
  /**
   * @param factory_ attribute factory.
   * @param reader_ input.
   */
  public MonoTokenizer(AttributeFactory factory_, Reader reader_) {
    super(Version.LUCENE_4_9, factory_, reader_);
  }
  
  @Override
  protected boolean isTokenChar(int i) {
    return true;
  }
}
