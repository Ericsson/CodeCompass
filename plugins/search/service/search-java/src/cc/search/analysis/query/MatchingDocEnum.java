package cc.search.analysis.query;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.apache.lucene.index.DocsAndPositionsEnum;
import org.apache.lucene.index.Term;
import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Helper class for enumerating possibly matching documents.
 * 
 * @author Alex Gábor Ispánovics <gabor.alex.ispanovics@ericsson.com>
 */
final class MatchingDocEnum {
  /**
   * Helper class for enumeration over term positions and storing the context of
   * a term.
   */
  static final class TermContext {
    /**
     * An extremal value for {@link TermContext#nextPosition()}.
     */
    static final int NO_MORE_POSITION = Integer.MAX_VALUE;
    
    /**
     * The terms itself.
     */
    private final Term _term;
    /**
     * Enumerator.
     */
    private final DocsAndPositionsEnum _termDocPosEnum;
    /**
     * The position of the term in the query.
     */
    private final int _relativePosition;
    /**
     * Number of remaining positions in the current document (
     * -1 if it is uninitialized).
     */
    private int _remainingPositions = -1;
    /**
     * The last value returned by nextPosition() (-1 if it is uninitialized).
     */
    private int _lastPosition = -1;

    /**
     * @param term_ a term.
     * @param termDocPosEnum_ an enumerator.
     * @param relativePosition_  the term`s position in the query.
     */
    TermContext(Term term_, DocsAndPositionsEnum termDocPosEnum_,
      int relativePosition_) {
      _term = term_;
      _termDocPosEnum = termDocPosEnum_;
      _relativePosition = relativePosition_;
    }
    
    /**
     * @see TermContext#_term
     * @return terms.
     */
    Term term() {
      return _term;
    }
    
    /**
     * The position of the term in the query.
     * 
     * @see TermContext#_relativePosition
     * @return relative position.
     */
    int relativePosition() {
      return _relativePosition;
    }
    
    /**
     * Sets the value of _remainingPositions. Must be called before any
     * nextPosition() call.
     * @throws IOException 
     */
    void startPositionIteration() throws IOException {
      _remainingPositions = _termDocPosEnum.freq();
      _lastPosition = -1;
    }
    
    /**
     * @return the next position in the current document or NO_MORE_POSITION.
     * @throws IOException 
     */
    int nextPosition() throws IOException {
      assert _remainingPositions >= 0 :"startPositionIteration must be called!";
      
      if (_remainingPositions > 0) {
        --_remainingPositions;
        _lastPosition = _termDocPosEnum.nextPosition();
      } else {
        _lastPosition = NO_MORE_POSITION;
      }
      
      return _lastPosition;
    }
    
    /**
     * @see TermContext#_lastPosition
     * @return last position.
     */
    int lastPosition() {
      return _lastPosition;
    }
    
    /**
     * The last document id returned by advance().
     * @return last doc id.
     */
    int lastDocId() {
      return _termDocPosEnum.docID();
    }
    
    /**
     * Returns the current start offset.
     * @return current start offset.
     * @throws IOException 
     */
    int startOffset() throws IOException {
      return _termDocPosEnum.startOffset();
    }
    
    /**
     * Returns the current end offset.
     * @return current end offset.
     * @throws IOException 
     */
    int endOffset() throws IOException {
      return _termDocPosEnum.endOffset();
    }
    
    /**
     * Advances to the next document by targetDocId_.
     * 
     * @param targetDocId_ target doc.
     * @return a doc id (or NO_MORE_DOCS).
     * @throws IOException 
     */
    int advance(int targetDocId_) throws IOException {
      _remainingPositions = -1;
      _lastPosition = -1;
      return _termDocPosEnum.advance(targetDocId_);
    }
  }
  
  /**
   * Helper class for a matching document.
   */
  static class DocMatch {
    /**
     * Document id.
     */
    private final int _docId;
    /**
     * Terms from the query found in this document.
     */
    private final List<TermContext> _matches;
    
    /**
     * @param docId_ document id.
     * @param matches_ matches.
     */
    DocMatch(int docId_, List<TermContext> matches_) {
      _docId = docId_;
      _matches = matches_;
    }
    
    /**
     * @return document id.
     */
    int docId() {
      return _docId;
    }
    
    /**
     * @return list of matching terms.
     */
    List<TermContext> matches() {
      return _matches;
    }
  }
  
  /**
   * Terms in the enumerator.
   */
  final ArrayList<TermContext> _terms;
  
  /**
   * @param termCapacity_ capacity for container allocation.
   */
  MatchingDocEnum(int termCapacity_) {
    _terms = new ArrayList<>(termCapacity_);
  }
  
  /**
   * Advance to the next lowest document id starting from targetDocId_.
   * 
   * @param targetDocId_ the lower bound of the next doc id.
   * @return A document match or null if no more.
   * @throws IOException 
   */
  DocMatch advance(int targetDocId_) throws IOException {
    int leastDoc = NO_MORE_DOCS;
    ArrayList<TermContext> matches = new ArrayList<>(_terms.size());
    
    for (final TermContext ctx : _terms) {
      int doc = ctx.lastDocId();
      // From the documentation: docID returns -1 or NO_MORE_DOCS if nextDoc()
      // or advance(int) were not called yet.
      if (doc == -1 || doc == NO_MORE_DOCS || doc < targetDocId_) {
        // Find next matching doc
        doc = ctx.advance(targetDocId_);
      }
      
      if (doc < leastDoc) {
        matches.clear();
        matches.add(ctx);
        leastDoc = doc;
      } else if (doc != NO_MORE_DOCS && doc == leastDoc) {
        matches.add(ctx);
      }
    }
    
    if (leastDoc == NO_MORE_DOCS) {
      return null;
    }
    
    return new DocMatch(leastDoc, matches);
  }
  
  /**
   * Add a context to the enumerator.
   * 
   * @param ctx_ a context.
   */
  void add(TermContext ctx_) {
    _terms.add(ctx_);
  }
  
  /**
   * Number of term contexts (terms) in this enumerator.
   * @return number of term contexts.
   */
  int numTermContext() {
    return _terms.size();
  }
}
