package cc.search.match.matcher;

import cc.search.analysis.tags.Tag;
import cc.search.analysis.tags.Tags;
import cc.search.common.IndexFields;
import cc.search.match.Context;
import cc.search.match.QueryContext;
import java.io.IOException;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.TokenFilter;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.highlight.TokenSources;

/**
 * Matcher factory for matching by tag kinds.
 */
class TagKindMatcherFactory implements ResultMatcherFactory {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger(TagKindMatcherFactory.
    class.getName());
  /**
   * Token filter for filtering relevant tokens by its kind.
   */
  private static class TagKindFilter extends TokenFilter {
    /**
     * Tags.
     */
    private final Tags _tags;
    /**
     * Allowed kinds.
     */
    private final Set<Tag.Kind> _kinds;
    /**
     * Current token offset.
     */
    private final OffsetAttribute _currentOffset;
    
    /**
     * @param stream_ a stream (with term vectors)
     * @param context_ searching context.
     * @param tags_ tags
     * @param kinds_ allowed kinds. Empty set means no filtering.
     */
    TagKindFilter(TokenStream stream_, Tags tags_, Set<Tag.Kind> kinds_) {
      super(stream_);
      
      _tags = tags_;
      _kinds = kinds_;
      
      _currentOffset = addAttribute(OffsetAttribute.class);
    }

    @Override
    public boolean incrementToken() throws IOException {
      while (input.incrementToken()) {
        final Tag tag = _tags.getByOffset(_currentOffset.startOffset());
        if (tag == null) {
          _log.log(Level.WARNING, "No tag find on offset: {0}",
            _currentOffset.startOffset());
          continue;
        }
        
        if (_kinds.isEmpty() || _kinds.contains(tag.genericKind)) {
          return true;
        }
      }
      
      return false;
    }
  }

  @Override
  public ResultMatcher create(Context context_) throws IOException {
    final Query query = context_.query.get(QueryContext.QueryType.Tag);
    if (query == null) {
      return null;
    }
    
    // Extracted kinds
    final Set<Tag.Kind> kinds = (Set<Tag.Kind>) context_.query.
      getData(QueryContext.QueryType.Tag);
    
    // Load terms
    Tags tags;
    try {
      tags = Tags.deserialize(context_.document.getBinaryValue(
        IndexFields.tagsField).bytes);
    } catch (ClassNotFoundException ex) {
      throw new IOException("Deserializing tags failed!", ex);
    }
    
    // Get the base stream.
    TokenStream stream = TokenSources.getAnyTokenStream(
      context_.searcher.getIndexReader(), context_.documentId,
      IndexFields.definitionsField, context_.document, null);
    
    // Filter the stream
    stream = new TagKindFilter(stream, tags, kinds);

    return new OffsetBasedLineMatcher(context_, query,
      IndexFields.definitionsField, stream) { };
  }

  @Override
  public void close() {
  }
}
