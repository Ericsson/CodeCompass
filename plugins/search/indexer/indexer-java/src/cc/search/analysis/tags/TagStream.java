package cc.search.analysis.tags;

import java.io.IOException;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.tokenattributes.CharTermAttribute;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;
import org.apache.lucene.search.BoostAttribute;

/**
 * TokenStream for Tags.
 */
public class TagStream extends TokenStream {
  /**
   * Tag filter interface.
   */
  public static interface Filter {
    /**
     * @param tag_
     * @return true if the given tag should be skipped, false otherwise.
     */
    boolean shouldSkip(Tag tag_);
  }
  /**
   * A filter which accepts everything.
   */
  public final static class NullFilter implements Filter {
    @Override
    public boolean shouldSkip(Tag tag_) {
      return false;
    }
  }
  /**
   * Tag kind filter.
   */
  public final static class KindFilter implements Filter {
    /**
     * Accepted kinds.
     */
    private final List<Tag.Kind> _acceptedKinds;
    /**
     * @param acceptedKinds_ accepted kinds.
     */
    public KindFilter(Tag.Kind[] acceptedKinds_) {
      _acceptedKinds = Arrays.asList(acceptedKinds_);
    }
    /**
     * @param acceptedKind_ accepted kind.
     */
    public KindFilter(Tag.Kind acceptedKind_) {
      this(new Tag.Kind[] { acceptedKind_ });
    }

    @Override
    public boolean shouldSkip(Tag tag_) {
      return !_acceptedKinds.contains(tag_.genericKind);
    }
  }
  
  /**
   * Offset attribute.
   */
  private final OffsetAttribute _offsetAttr
    = addAttribute(OffsetAttribute.class);
  /**
   * Char term attribute.
   */
  private final CharTermAttribute _charAttr
    = addAttribute(CharTermAttribute.class);
  /**
   * Boost for this tag.
   */
  private final BoostAttribute _boostAttr = addAttribute(BoostAttribute.class);
  
  /**
   * Parsed tags.
   */
  private final Tags _tags;
  /**
   * Tag filter.
   */
  private final Filter _filter;
  /**
   * Iterator for iterating over the parsed tags.
   */
  private Iterator<Tags.TagItem> _tagIterator;
  
  /**
   * @param tags_ tags.
   * @param filter_ tag filter.
   */
  public TagStream(Tags tags_, Filter filter_) {
    _tags = tags_;
    _filter = filter_;
  }

  @Override
  public boolean incrementToken() throws IOException {
    clearAttributes();
    
    while (_tagIterator.hasNext()) {
      final Tags.TagItem tagItem = _tagIterator.next();

      if (_filter.shouldSkip(tagItem.tag)) {
        continue;
      }
      
      final int startOffset = tagItem.offset;
      _offsetAttr.setOffset(startOffset, startOffset +
        tagItem.tag.location.endColumn - tagItem.tag.location.startColumn + 1);
      _charAttr.append(tagItem.tag.text.toLowerCase());
      
      switch (tagItem.tag.genericKind) {
        case Type:
          _boostAttr.setBoost(2.0f);
          break;
        case Function:
          _boostAttr.setBoost(1.8f);
          break;
        case Field:
          _boostAttr.setBoost(1.5f);
          break;
        default:
          _boostAttr.setBoost(1.0f);
          break;
      }
      
      return true;
    }
    
    return false;
  }
  
  @Override
  public void reset() throws IOException {
    super.reset();
    _tagIterator = _tags.getAllTags().iterator();
  }
  
  @Override
  public void end() throws IOException {
    super.end();
    _tagIterator = null;
  }
  
  @Override
  public void close() throws IOException {
    super.close();
    _tagIterator = null;
  }
}
