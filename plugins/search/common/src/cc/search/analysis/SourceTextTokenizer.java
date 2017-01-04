package cc.search.analysis;

import java.io.Reader;
import org.apache.lucene.analysis.util.CharTokenizer;
import org.apache.lucene.util.AttributeFactory;
import org.apache.lucene.util.Version;

/**
 * Tokenizer for source files.
 */
class SourceTextTokenizer extends CharTokenizer {
  /**
   * @param input_ input.
   */
  SourceTextTokenizer(Reader input_) {
    super(Version.LUCENE_4_9, input_);
  }
  
  /**
   * @param factory_ attribute factory.
   * @param input_ input.
   */
  SourceTextTokenizer(AttributeFactory factory_, Reader input_) {
    super(Version.LUCENE_4_9, factory_, input_);
  }

  @Override
  protected boolean isTokenChar(int i) {
    if (Character.isWhitespace(i)) {
      return false;
    }
    
    int type = Character.getType(i);
    switch (type) {
      case Character.UPPERCASE_LETTER:
      case Character.LOWERCASE_LETTER:
      case Character.TITLECASE_LETTER:
      case Character.MODIFIER_LETTER:
      case Character.OTHER_LETTER:
      case Character.DECIMAL_DIGIT_NUMBER:
      case Character.CONNECTOR_PUNCTUATION:
        return true;
      default:
        return '#' == i;
    } 
  }
}
