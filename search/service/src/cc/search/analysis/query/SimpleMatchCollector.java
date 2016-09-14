package cc.search.analysis.query;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

/**
 * A simple implementation of MatchCollector that stores all matches.
 * 
 * @author Alex Gábor Ispánovics <gabor.alex.ispanovics@ericsson.com>
 */
public class SimpleMatchCollector implements MatchCollector {
  /**
   * Document id -> MatchInfo multimap.
   */
  private final HashMap<Integer, ArrayList<MatchInfo>> _matches
    = new HashMap<>();
  /**
   * The maximal allowed difference from the best score per document.
   * So 0.0 means it will only collects the best matches(s). If it has a
   * negative value, then will be no checking (accepts all matches).
   */
  private float _maxDiff ;
  
  /**
   * Creates a new SimpleMatchCollector with -1 as maximal difference.
   */
  public SimpleMatchCollector() {
    this(-1);
  }
  
  /**
   * Creates a new SimpleMatchCollector with the given maximal difference.
   * 
   * @param maximumDifference_ maximal difference.
   */
  public SimpleMatchCollector(float maximumDifference_) {
    _maxDiff = maximumDifference_;
  }
  
  /**
   * Drops all elements that beyond the maximum difference.
   * 
   * @param infos_ container to clean.
   */
  private void cleanupMatches(ArrayList<MatchInfo> infos_) {
    if (_maxDiff < 0) {
      return;
    }
    
    MatchInfo largest = infos_.get(0);
    Iterator<MatchInfo> iter = infos_.iterator();
    while (iter.hasNext()) {
      if ((largest.score - iter.next().score) > _maxDiff) {
        iter.remove();
      }
    }
  }
  
  @Override
  public void collectMatch(int docId_, MatchInfo info_) {
    ArrayList<MatchInfo> infos = _matches.get(docId_);
    if (infos != null) {
      MatchInfo largest = infos.get(0);
      if (_maxDiff >= 0 && (largest.score - info_.score) > _maxDiff) {
        return;
      }
      
      if (info_.score > largest.score) {
        infos.add(0, info_);
        cleanupMatches(infos);
      } else {
        infos.add(info_);
      }
    } else {
      infos = new ArrayList<>();
      infos.add(info_);
      _matches.put(docId_, infos);
    }
  }
  
  /**
   * Returns the collected matches for a document. 
   * 
   * @param docId_ document id.
   * @return the list of matches in the document or null.
   */
  public List<MatchInfo> getResultsInDoc(int docId_) {
    return _matches.get(docId_);
  }
  
  /**
   * Setter for maximal difference. See {@link #_maxDiff}.
   * 
   * @param maxDiff_ new maximal difference.
   */
  public void setMaximalDifference(float maxDiff_) {
    _maxDiff = maxDiff_;
    
    for (ArrayList<MatchInfo> infos : _matches.values()) {
      cleanupMatches(infos);
    }
  }
}
