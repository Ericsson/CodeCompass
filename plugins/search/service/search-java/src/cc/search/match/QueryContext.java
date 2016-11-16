package cc.search.match;

import cc.search.common.IndexFields;
import java.util.HashMap;
import java.util.Map;
import org.apache.lucene.queries.CustomScoreQuery;
import org.apache.lucene.queries.function.FunctionQuery;
import org.apache.lucene.queries.function.valuesource.LongFieldSource;
import org.apache.lucene.search.BooleanClause;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Query;

/**
 * Context class for query parsing/matching.
 */
public class QueryContext {
  /**
   * Type of query.
   */
  public static enum QueryType {
    /**
     * Text search query.
     */
    Text,
    /**
     * Tag search (aka "Advanced search") query.
     */
    Tag,
    /**
     * Log entry search.
     */
    Log
  }
  /**
   * Map from QueryType to a query.
   */
  private final Map<QueryType, Query> _queries;
  /**
   * Map from QueryType to a query data.
   */
  private final Map<QueryType, Object> _queryData;
  /**
   * Final query (used to get the matching documents). 
   */
  private final BooleanQuery _finalQuery;
  /**
   * Filter overlapping result lines.
   */
  private boolean _filterOverlapping = false;
  /**
   * Creates an empty context.
   */
  public QueryContext() {
    _queries = new HashMap<>();
    _queryData = new HashMap<>();
    _finalQuery = new BooleanQuery();
  }
  /**
   * Adds a query part to the context. If a query with the same type is already
   * exists, than an IllegalArgumentException will be thrown.
   * 
   * @param type_ query type.
   * @param query_ query.
   * @param data_ custom data
   * @throws IllegalArgumentException
   */
  public void add(QueryType type_, Query query_, Object data_) {
    if (_queries.containsKey(type_)) {
      throw new IllegalArgumentException(type_.toString() + " already added");
    } else {
      _queries.put(type_, query_);
      _queryData.put(type_, data_);
      _finalQuery.add(query_, BooleanClause.Occur.SHOULD);
    }
  }
  /**
   * Adds a query part to the context. If a query with the same type is already
   * exists, than an IllegalArgumentException will be thrown.
   * 
   * @param type_ query type.
   * @param query_ query.
   * @throws IllegalArgumentException
   */
  public void add(QueryType type_, Query query_) {
    add(type_, query_, null);
  }
  /**
   * Returns a query for getting the documents.
   * 
   * @return the final query.
   */
  public Query get() {
    if (_queries.containsKey(QueryType.Log)) {
      // Log query very sensitive to its score so no boosting in this case.
      return _finalQuery;
    } else {
      return new CustomScoreQuery(
        _finalQuery,
        new FunctionQuery(new LongFieldSource(IndexFields.boostValue)));
    }
  }
  /**
   * Returns a query (previously added by add) for the given query type.
   * 
   * @param type_ query type.
   * @return query or null if no query exists for the given type.
   */
  public Query get(QueryType type_) {
    return _queries.get(type_);
  }
  /**
   * Returns the custom data associated to the query.
   * 
   * @param type_ query type.
   * @return custom data or null if no query exists for the given type.
   */
  public Object getData(QueryType type_) {
    return _queryData.get(type_);
  }
  /**
   * @return true if the context is empty.
   */
  public boolean isEmpty() {
    return _queries.isEmpty();
  }
  /**
   * Getter for {@link QueryContext#_filterOverlapping}.
   * 
   * @return {@link QueryContext#_filterOverlapping}
   */
  public boolean getFilterOverlapping() {
    return _filterOverlapping;
  }
  /**
   * Setter for {@link QueryContext#_filterOverlapping}.
   * 
   * @param value_ value for {@link QueryContext#_filterOverlapping}.
   */
  public void setFilterOverlapping(boolean value_) {
    _filterOverlapping = value_;
  }
}
