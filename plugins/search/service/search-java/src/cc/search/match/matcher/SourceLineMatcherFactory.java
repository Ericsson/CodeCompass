package cc.search.match.matcher;

import cc.search.common.IndexFields;
import cc.search.match.Context;
import cc.search.match.QueryContext;
import java.io.IOException;
import org.apache.lucene.search.Query;

/**
 * ResultMatcherFactory for SourceLineMatcher.
 */
class SourceLineMatcherFactory implements ResultMatcherFactory {
  @Override
  public ResultMatcher create(Context context_) throws IOException {
    final Query query = context_.query.get(QueryContext.QueryType.Text);
    if (query == null) {
      return null;
    } else {
      return OffsetBasedLineMatcher.fromTermVectorField(context_, query,
        IndexFields.contentField);
    }
  }

  @Override
  public void close() {
  }
}
