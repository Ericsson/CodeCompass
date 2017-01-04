package cc.search.analysis.log;

import cc.search.analysis.SourceTextAnalyzer;
import cc.search.analysis.query.HIPDQuery;
import cc.search.analysis.query.MatchCollector;
import cc.search.common.IndexFields;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.tokenattributes.CharTermAttribute;
import org.apache.lucene.index.Term;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Query;

/**
 * Query builder for log search queries.
 */
public class LogQueryBuilder {
  /**
   * Analyzer for token extraction.
   */
  private final Analyzer _analyzer = new SourceTextAnalyzer();
  
  /**
   * Construct a new LogQueryBuilder.
   */
  public LogQueryBuilder() {
  }
  
  /**
   * Analyzes the given query and collect the tokens form it.
   *  
   * @param queryText_ query text
   * @return tokens in order.
   * @throws IOException 
   */
  private List<String> collectTokens(String queryText_) throws IOException {
    final ArrayList<String> resultTokens = new ArrayList<>();
    
    try (final TokenStream tokenStream = _analyzer.tokenStream(
      IndexFields.contentField, queryText_)) {
      
      tokenStream.reset();
      
      CharTermAttribute tok = tokenStream.addAttribute(CharTermAttribute.class);
      while (tokenStream.incrementToken()) {
        resultTokens.add(tok.toString());
      }
      
      tokenStream.end();
    }
    
    return resultTokens;
  }
  
  /**
   * Build a Lucene query from a user query text.
   * 
   * @param queryText_ query text from a user.
   * @param collector_ result collector;
   * @return a representing query.
   * @throws java.io.IOException
   */
  public Query build(String queryText_,
    MatchCollector collector_) throws IOException {
    List<String> tokens = collectTokens(queryText_);
    if (tokens.size() < HIPDQuery.DEFAULT_MIN_MATCHING_TERM) {
      return new BooleanQuery();
    }
    
    HIPDQuery query = new HIPDQuery();
    query.setResultCollector(collector_);
    query.setAllowSwappedTerms(true);
    for (final String token : tokens) {
      query.add(new Term(IndexFields.contentField, token));
    }
    
    return query;
  }
  
  /**
   * Build a Lucene query from a user query text.
   * 
   * @param queryText_ query text from a user.
   * @return a representing query.
   * @throws java.io.IOException
   */
  public Query build(String queryText_) throws IOException {
    return build(queryText_, null);
  }
}
