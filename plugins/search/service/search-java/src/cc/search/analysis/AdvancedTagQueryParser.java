package cc.search.analysis;

import cc.search.analysis.tags.Tag;
import cc.search.common.IndexFields;
import java.util.HashSet;
import java.util.Set;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryparser.classic.ParseException;
import org.apache.lucene.queryparser.classic.QueryParser;
import org.apache.lucene.search.Query;
import org.apache.lucene.util.Version;

/**
 * Query parser for advanced tag search.
 */
public class AdvancedTagQueryParser extends QueryParser {
  /**
   * Set of kinds found in the last parsed query.
   */
  private final Set<Tag.Kind> _tagKinds;
  
  /**
   * Creates an AdvancedTagQueryParser.
   * 
   * @param analyzer_ an analyzer.
   */
  public AdvancedTagQueryParser(Analyzer analyzer_) {
    super(Version.LUCENE_4_9, IndexFields.definitionsField, analyzer_);
    
    _tagKinds = new HashSet<>();
  }
  
  /**
   * Returns the kinds found in the query during a parse() call.
   * 
   * @return set of kinds.
   */
  public Set<Tag.Kind> getParsedKinds() {
    return _tagKinds;
  }
  
  /**
   * Adds the given field to the tag kind set it is a tag kind field.
   * 
   * @param field_ a field.
   */
  private void tryAddKindFromField(String field_) {
    try {
      _tagKinds.add(IndexFields.getTagKindForFieldName(field_));
    } catch (IllegalArgumentException ex) {
      // This is not a "kind" field. Nothing to worry about :-)
    }
  }

  @Override
  protected Query newWildcardQuery(Term term_) {
    tryAddKindFromField(term_.field());
    return super.newWildcardQuery(term_);
  }

  @Override
  protected Query newRangeQuery(String field_, String p1_, String p2_,
    boolean startInclusive_, boolean endInclusive_) {
    tryAddKindFromField(field_);
    return super.newRangeQuery(field_, p1_, p2_, startInclusive_,
      endInclusive_);
  }

  @Override
  protected Query newFuzzyQuery(Term term_, float minimumSimilarity_,
    int prefixLength_) {
    tryAddKindFromField(term_.field());
    return super.newFuzzyQuery(term_, minimumSimilarity_, prefixLength_);
  }

  @Override
  protected Query newRegexpQuery(Term regexp_) {
    tryAddKindFromField(regexp_.field());
    return super.newRegexpQuery(regexp_);
  }

  @Override
  protected Query newPrefixQuery(Term prefix_) {
    tryAddKindFromField(prefix_.field());
    return super.newPrefixQuery(prefix_);
  }

  @Override
  protected Query newFieldQuery(Analyzer analyzer_, String field_,
    String queryText_, boolean quoted_) throws ParseException {
    tryAddKindFromField(field_);
    return super.newFieldQuery(analyzer_, field_, queryText_, quoted_);
  }

  @Override
  public Query parse(String query_) throws ParseException {
    _tagKinds.clear();
    return super.parse(query_);
  }
}
