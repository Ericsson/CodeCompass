package cc.search.analysis.query;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.index.AtomicReader;
import org.apache.lucene.index.AtomicReaderContext;
import org.apache.lucene.index.DocsAndPositionsEnum;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexReaderContext;
import org.apache.lucene.index.Term;
import org.apache.lucene.index.TermContext;
import org.apache.lucene.index.TermState;
import org.apache.lucene.index.Terms;
import org.apache.lucene.index.TermsEnum;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Explanation;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Scorer;
import org.apache.lucene.search.TermQuery;
import org.apache.lucene.search.TermStatistics;
import org.apache.lucene.search.Weight;
import org.apache.lucene.search.similarities.Similarity;
import org.apache.lucene.util.Bits;

/**
 * Hyper intelligent pan dimensional query.
 * 
 * It is a multi-term/phrase query with some unique features:
 *   - all term in the query is optional, but there is a minimal number of terms
 *     that should match (see: {@link HIPDQuery#setMinMatchingTerms(int)}). The
 *     default value is {@link HIPDQuery#DEFAULT_MIN_MATCHING_TERM}.
 *   - The terms has an order in the query (like a phrase query) but swapping
 *     them also enabled with {@link HIPDQuery#setAllowSwappedTerms(boolean)}.
 *   - Allows other terms between searched terms up to a given number. The
 *     default is {@link HIPDQuery#DEFAULT_MAX_DIFFERENCE} but you can override
 *     the default value with {@link HIPDQuery#setMaxDifference(int)}.
 * 
 * @author Alex Gábor Ispánovics <gabor.alex.ispanovics@ericsson.com>
 */
public class HIPDQuery extends Query {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * The default value of the maximal allowed difference.
   */
  public final static int DEFAULT_MAX_DIFFERENCE = 20;
  /**
   * The default value of the minimal number of terms.
   */
  public final static int DEFAULT_MIN_MATCHING_TERM = 2;
  /**
   * An optional result collector.
   */
  private MatchCollector _resultCollector = null;
  /**
   * @see MatchScorer#_maxDiff.
   */
  private int _maxDifference = DEFAULT_MAX_DIFFERENCE;
  /**
   * @see MatchScorer#_minTerms.
   */
  private int _minMatchingTerm = DEFAULT_MIN_MATCHING_TERM;
  /**
   * @see MatchScorer#_allowSwap.
   */
  private boolean _allowSwappedTerms = false;
  /**
   * Terms in the query.
   */
  private final ArrayList<Term> _terms = new ArrayList<>();
  /**
   * A document field for terms.
   */
  private String _field;
  
  /**
   * Lucene Weight implementation for the query.
   */
  private class HIPDWeight extends Weight {
    /**
     * Some stuff from lucene.
     */
    private final Similarity _similarity;
    /**
     * Some more stuff from lucene.
     */
    private final Similarity.SimWeight _stats;
    /**
     * Lucene term contexts.
     */
    private final TermContext[] _termContext;
    
    /**
     * @param searcher_ a searcher.
     * @throws IOException 
     */
    HIPDWeight(IndexSearcher searcher_) throws IOException {
      _similarity = searcher_.getSimilarity();
      
      final IndexReaderContext indexCtx = searcher_.getTopReaderContext();
      final TermStatistics termStats[] = new TermStatistics[_terms.size()];
      
      _termContext = new TermContext[_terms.size()];
      for (int i = 0; i < _terms.size(); i++) {
        final Term term = _terms.get(i);
        _termContext[i] = TermContext.build(indexCtx, term);
        termStats[i] = searcher_.termStatistics(term, _termContext[i]);
      }
      
      _stats = _similarity.computeWeight(getBoost(),
        searcher_.collectionStatistics(_field), termStats);
    }

    @Override
    public Explanation explain(AtomicReaderContext arc, int i)
      throws IOException {
      throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public Query getQuery() {
      return HIPDQuery.this;
    }

    @Override
    public float getValueForNormalization() throws IOException {
      return _stats.getValueForNormalization();
    }

    @Override
    public void normalize(float queryNorm_, float topLevelBoost_) {
      _stats.normalize(queryNorm_, topLevelBoost_);
    }

    @Override
    public Scorer scorer(AtomicReaderContext ctx_, Bits acceptDocs_)
      throws IOException {
      
      final AtomicReader reader = ctx_.reader();
      final Terms fieldTerms = reader.terms(_field);
      if (fieldTerms == null) {
        // Field does not exists, no problem.
        _log.log(Level.FINER, "Field does not exists!");
        return null;
      }
      
      final TermsEnum termEnum = fieldTerms.iterator(null);
      final MatchingDocEnum docEnum = new MatchingDocEnum(_terms.size());
      
      // Collect possibly matching terms
      for (int i = 0; i < _terms.size(); ++i) {
        final TermState termState = _termContext[i].get(ctx_.ord);
        if (termState == null) {
          // term not found in this segment, skip
          continue;
        }
        
        final Term term = _terms.get(i);
        
        // Seek to the term in the term dictionary
        termEnum.seekExact(term.bytes(), termState);
        
        // Get document and positions for the term
        DocsAndPositionsEnum docPosEnum = termEnum.docsAndPositions(acceptDocs_,
          null, DocsAndPositionsEnum.FLAG_OFFSETS);
        if (docPosEnum == null) {
          // Unlikely case: the field was indexed without position data
          throw new IllegalStateException("Field \"" + term.field() +
            "\" was indexed without position data! " +
            "Cannot run this type of query (term=" + term.text() + ")");
        }
        
        docEnum.add(new MatchingDocEnum.TermContext(term, docPosEnum, i));
      }
      
      if (docEnum.numTermContext() < _minMatchingTerm) {
        // Not enough term in this segment
        _log.log(Level.FINER, "Not enough matching term in this segment!");
        return null;
      }
      
      return new HIPDScorer(this, docEnum, ctx_.docBase);
    }

    @Override
    public boolean scoresDocsOutOfOrder() {
      return false;
    }
  }
  
  /**
   * Scorer implementation for this query.
   */
  private class HIPDScorer extends Scorer {
    /**
     * Helper class for searching the best match(es) in a document.
     */
    private final class BestMatchSearcher {
      /**
       * Current position -> term mapping.
       */
      private final TreeMap<Integer, MatchingDocEnum.TermContext>
        _availablePositions = new TreeMap<>();;
      /**
       * The current best match (if any).
       */
      private List<MatchingDocEnum.TermContext> _bestMatch = null;
      /**
       * The score of the best match (valid if _bestMatch is not null).
       */
      private float _bestMatchScore = 0;
      
      /**
       * Construct a new context for finding the best match in the document.
       * 
       * @param matchingTerms_ matching terms in the document.
       * @throws IOException 
       */
      private BestMatchSearcher(
        List<MatchingDocEnum.TermContext> matchingTerms_)
        throws IOException {
        ArrayList<MatchingDocEnum.TermContext> shiftTerms =
          new ArrayList<>(matchingTerms_);
        
        shiftPositions(shiftTerms);
      }
      
      /**
       * Shifts the positions of the terms in _availablePositions by
       * shiftTerms_. It also handles the collisions caused by terms with
       * multiple occurrence.
       * 
       * @parm shiftTerms_ list of terms that needs to be shifted.
       * @return false if we have no opportunity to find a better match, true
       * otherwise.
       * @throws IOException 
       */
      private boolean shiftPositions(
        List<MatchingDocEnum.TermContext> shiftTerms_) throws IOException {
        final ArrayList<MatchingDocEnum.TermContext> collisions =
          new ArrayList<>(shiftTerms_.size());

        for (final MatchingDocEnum.TermContext ctx : shiftTerms_) {
          // Shift terms must be dropped out from the available positions before
          // any shifting
          if (_availablePositions.get(ctx.lastPosition()) == ctx) {
            _availablePositions.remove(ctx.lastPosition());
          }
        }

        int availShiftTerms = shiftTerms_.size();
        for (final MatchingDocEnum.TermContext ctx : shiftTerms_) {
          int pos = ctx.nextPosition();
          if (pos == MatchingDocEnum.TermContext.NO_MORE_POSITION) {
            // drop this term
            if (_bestMatch != null && _bestMatch.size() >
              _availablePositions.size() + availShiftTerms) {
              // After we drop this term, we can`t find a better match, so better
              // return now.
              return false;
            } else {
              --availShiftTerms;
              // Continue with reduced term set.
              continue;
            }
          }
          
          final MatchingDocEnum.TermContext collision =
            _availablePositions.get(pos);
          if (collision != null) {
            // Multiple terms in the same position is not allowed, but if the
            // same term could be in the search expression in multiple times
            assert collision.term().equals(ctx.term()) :
              "Multiple terms in the same position is not allowed!";
            
             if (ctx.relativePosition() > collision.relativePosition()) {
              // Shift the current context because it should be after $collision
              // (if the order is relevant).
              collisions.add(ctx);
            } else {
              // Shift the one that currently in the map.
              collisions.add(collision);
              _availablePositions.remove(pos);
              _availablePositions.put(pos, ctx);
            }
          } else {
            --availShiftTerms;
            _availablePositions.put(pos, ctx);
          }
        }

        shiftTerms_.clear();
        shiftTerms_.addAll(collisions);
        if (!shiftTerms_.isEmpty()) {
          return shiftPositions(shiftTerms_);
        }

        return true;
      }
      
      /**
       * Finds a next possible match starting from the lowest position from
       * {@link #_availablePositions}.
       * 
       * @parm shiftTerms_ a collection for storing terms that needs to be
       *  shifted.
       * @return the list of matching terms or null.
       */
      private List<MatchingDocEnum.TermContext> findNextMatch(
        List<MatchingDocEnum.TermContext> shiftTerms_) {
        final int startPos = _availablePositions.firstKey();    
        int endPos = startPos;

        // Find the longest match with the least start position
        int currPos = _availablePositions.floorKey(endPos + _maxDifference);
        while (currPos != endPos) {
          endPos = currPos;
          currPos = _availablePositions.floorKey(endPos + _maxDifference);
        }

        final SortedMap<Integer, MatchingDocEnum.TermContext> matchCandidate =
          _availablePositions.subMap(startPos, endPos + 1);

        final MatchingDocEnum.TermContext leftmost =
          matchCandidate.get(matchCandidate.firstKey());
        if (matchCandidate.size() < _minMatchingTerm) {
          // If the longest match is too short, than we need to shift
          shiftTerms_.add(leftmost);
          return null;
        }

        final ArrayList<MatchingDocEnum.TermContext> match =
          new ArrayList<>(matchCandidate.size());
        if (_allowSwappedTerms) {
          // We allow swap so this is a match, no need to check the order
          match.addAll(matchCandidate.values());
        } else {
          // We have to select the terms in correct order
          int currentRelPos = leftmost.relativePosition();

          final Collection<MatchingDocEnum.TermContext> terms =
            matchCandidate.values();
          for (MatchingDocEnum.TermContext ctx : terms) {
            if (ctx.relativePosition() >= currentRelPos) {
              match.add(ctx);
              currentRelPos = ctx.relativePosition();
            }
          }
        }

        shiftTerms_.add(leftmost);

        return match;
      }
      
      /**
       * Calculates the score of a match.
       * 
       * @param match_ list of matching terms.
       * @return match score.
       */
      private float calculateMatchScore(
        List<MatchingDocEnum.TermContext> match_) {
        
        double similarity = 0;
        for (int i = 0; i < match_.size(); ++i) {
          for (int j = i + 1; j < match_.size(); ++j) {
            final MatchingDocEnum.TermContext mai = match_.get(i);
            final MatchingDocEnum.TermContext maj = match_.get(j);

            similarity += Math.pow(
              // P_a(a_j) -  P_a(a_i)
              (maj.lastPosition() - mai.lastPosition()) -
              // P_q(a_j) -  P_q(a_i)
              (maj.relativePosition() - mai.relativePosition()), 2);
          }
        }

        similarity /=
          // (N * (N - 1)) / 2
          (0.5 * (match_.size() * (match_.size() - 1))) *
          // ((|Q| - 1)(Dmax + 1))^2
          Math.pow((_terms.size() - 1) * (_maxDifference + 1), 2);

        similarity = 1.0 - similarity;

        return (float)(match_.size() + similarity);
      }
      
      /**
       * Finds the best match by the matching terms in a document and returns
       * its score. It will also call the result collector (if it is not null).
       * 
       * @param matchingTerms_ matching terms in the document.
       * @return the score of the best match in this document.
       * @throws IOException 
       */
      float findBestMatch() throws IOException {
        
        ArrayList<MatchingDocEnum.TermContext> shiftTerms =
          new ArrayList<>(_docEnum.numTermContext());

        while (_availablePositions.size() >= _minMatchingTerm &&
          (_bestMatch == null ||
           _bestMatch.size() <= _availablePositions.size())) {

          List<MatchingDocEnum.TermContext> match = findNextMatch(shiftTerms);
          if (match != null && match.size() >= _minMatchingTerm) {
            float matchScore = calculateMatchScore(match);

            if (matchScore > 0 && _resultCollector != null) {
              final MatchingDocEnum.TermContext left = match.get(0);
              final MatchingDocEnum.TermContext right = match.get(
                match.size() - 1);
              
              _resultCollector.collectMatch(left.lastDocId() + _docBase,
                new MatchCollector.MatchInfo(matchScore, left.startOffset(),
                  right.endOffset()));
            }

            if (matchScore > _bestMatchScore) {
              _bestMatchScore = matchScore;
              _bestMatch = match;
            }
          }

          if (!shiftPositions(shiftTerms)) {
            break;
          }
        }

        return _bestMatchScore;
      }
    }
    
    /**
     * The current document id.
     */
    private int _currentDoc = -1;
    /**
     * The current document score.
     */
    private float _currentDocScore = 1;
    /**
     * A document enumerator.
     */
    private final MatchingDocEnum _docEnum;
    /**
     * The document offset from the reader context.
     */
    private final int _docBase;
    
    /**
     * @param weight_ the weight that creates this class.
     * @param docEnum_ a document enumerator.
     * @param docBase_ the document offset from the reader context.
     */
    HIPDScorer(HIPDWeight weight_, MatchingDocEnum docEnum_, int docBase_) {
      super(weight_);
      _docEnum = docEnum_;
      _docBase = docBase_;
    }
    
    /**
     * Calculates the score of a document match.
     * 
     * @param docMatch_ a matching document.
     * @return the score.
     * @throws IOException 
     */
    private float calculateScore(MatchingDocEnum.DocMatch docMatch_)
      throws IOException {
      final List<MatchingDocEnum.TermContext> matches = docMatch_.matches();
      if (matches.size() < _minMatchingTerm) {
        return 0;
      }
      
      for (MatchingDocEnum.TermContext ctx : matches) {
        ctx.startPositionIteration();
      }
      
      BestMatchSearcher searcher = new BestMatchSearcher(matches);
      return searcher.findBestMatch();
    }

    @Override
    public float score() throws IOException {
      return _currentDocScore;
    }

    @Override
    public int freq() throws IOException {
      // Only the best match counts.
      return 1;
    }

    @Override
    public int docID() {
      return _currentDoc;
    }

    @Override
    public int nextDoc() throws IOException {
      if (docID() != NO_MORE_DOCS) {
        return advance(docID() + 1);
      }
      
      return NO_MORE_DOCS;
    }

    @Override
    public int advance(int targetDocId_) throws IOException {  
      _currentDoc = NO_MORE_DOCS;
      _currentDocScore = 0;
      
      MatchingDocEnum.DocMatch candidateDoc = _docEnum.advance(targetDocId_);
      while (candidateDoc != null) {
        final float score = calculateScore(candidateDoc);
        if (score > 0) {
          _currentDoc = candidateDoc.docId();
          _currentDocScore = score;
          break;
        }
        
        candidateDoc = _docEnum.advance(candidateDoc.docId() + 1);
      }
      
      return _currentDoc;
    }

    @Override
    public long cost() {
      // I don`t know better.
      return 42;
    }
  }
  
  /**
   * Sets the maximal term difference.
   * 
   * @see HIPDQuery#_maxDifference.
   * @param maxDifference_  maximal term difference.
   */
  public void setMaxDifference(int maxDifference_) {
    assert maxDifference_ >= 0;
    _maxDifference = maxDifference_;
  }
  
  /**
   * Sets the value of {@link HIPDQuery#_allowSwappedTerms}.
   * 
   * @see HIPDQuery#_allowSwappedTerms.
   * @param allowSwappedTerms_ your choice.
   */
  public void setAllowSwappedTerms(boolean allowSwappedTerms_) {
    _allowSwappedTerms = allowSwappedTerms_;
  }
  
  /**
   * Sets the minimum of needed terms.
   * 
   * @see HIPDQuery#_minMatchingTerm.
   * @param minMatchingTerm_ minimally needed terms.
   */
  public void setMinMatchingTerms(int minMatchingTerm_) {
    assert minMatchingTerm_ >= 2;
    
    if (minMatchingTerm_ >= 2)
    {
      _minMatchingTerm = minMatchingTerm_;
    }
  }
  
  /**
   * Sets a result collector.

   * @param resultCollector_ a collector.
   */
  public void setResultCollector(MatchCollector resultCollector_) {
    _resultCollector = resultCollector_;
  }
  
  /**
   * Add a term to the query. All term must be in the same field!

   * @param term_ a term.
   */
  public void add(Term term_) {    
    if (_terms.isEmpty()) {
      _field = term_.field();
    } else if (!_field.equals(term_.field())) {
      assert false : "All terms must be in the same field!";
    }

    _terms.add(term_);
  }
  
  @Override
  public Weight createWeight(IndexSearcher is_) throws IOException {
    return new HIPDWeight(is_);
  }

  @Override
  public String toString() {
    return "HIPDQuery{" + 
        "max difference=" + _maxDifference +
      ", min matching terms=" + _minMatchingTerm +
      ", allow swapped terms=" + _allowSwappedTerms +
      ", terms=" + _terms +
      ", field=" + _field + '}';
  }
  
  @Override
  public String toString(String field_) {
    if (field_.equals(_field)) {
      return toString();
    } else {
      return "HIPDQuery N/A";
    }
  }

  @Override
  public void extractTerms(Set<Term> terms_) {
    terms_.addAll(_terms);
  }
  
  @Override
  public Query rewrite(IndexReader reader_) throws IOException {
    assert _terms.size() < _minMatchingTerm : "Not enough term in the query!";
    
    Query query = null;
    if (_terms.isEmpty()) {
      query = new BooleanQuery();
    } else if (_terms.size() == 1) {
      query = new TermQuery(_terms.get(0));
    }
    
    if (query != null) {
      query.setBoost(getBoost());
      return query;
    }
    
    return super.rewrite(reader_);
  }

  @Override
  public int hashCode() {
    int hash = 3;
    hash = 71 * hash + this._maxDifference;
    hash = 71 * hash + this._minMatchingTerm;
    hash = 71 * hash + (this._allowSwappedTerms ? 1 : 0);
    hash = 71 * hash + Objects.hashCode(this._terms);
    hash = 71 * hash + Objects.hashCode(this._field);
    return hash;
  }

  @Override
  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    if (getClass() != obj.getClass()) {
      return false;
    }
    final HIPDQuery other = (HIPDQuery) obj;
    if (this._maxDifference != other._maxDifference) {
      return false;
    }
    if (this._minMatchingTerm != other._minMatchingTerm) {
      return false;
    }
    if (this._allowSwappedTerms != other._allowSwappedTerms) {
      return false;
    }
    if (!Objects.equals(this._terms, other._terms)) {
      return false;
    }
    return Objects.equals(this._field, other._field);
  }
}
