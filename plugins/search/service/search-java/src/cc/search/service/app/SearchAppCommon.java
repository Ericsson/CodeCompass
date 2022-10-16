package cc.search.service.app;

import cc.search.analysis.QueryAnalyzer;
import cc.search.common.config.CommonOptions;
import cc.search.common.IndexFields;
import cc.search.match.Context;
import cc.search.match.QueryContext;
import cc.search.match.matcher.MasterMatcherFactory;
import cc.search.match.matcher.ResultMatcher;
import cc.search.match.matcher.ResultMatcherFactory;
import cc.service.core.FileInfo;
import cc.service.search.LineMatch;
import cc.service.core.Range;
import cc.service.search.SearchResultEntry;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.DirectoryReader;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.queryparser.classic.QueryParser;
import org.apache.lucene.search.Filter;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.ScoreDoc;
import org.apache.lucene.search.TopDocs;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.util.Version;

/**
 * Abstract base class for Search Application and Search Service.
 */
public abstract class SearchAppCommon implements AutoCloseable {
  /**
   * Default hit limit.
   */
  public static final int DEFAULT_HIT_LIMIT = 100;
  /**
   * Program options.
   */
  protected final CommonOptions _options;
  /**
   * Index database.
   */
  protected final IndexReader _indexReader;
  /**
   * The once and only searcher.
   */
  protected final IndexSearcher _searcher;
  /**
   * Parser for query strings.
   */
  protected final QueryParser _textQueryParser;
  /**
   * Executor for async tasks.
   */
  protected final ExecutorService _executor;
  /**
   * Matcher factory.
   */
  protected final ResultMatcherFactory _matcherFactory;
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger("GLOBAL_LOGGER");
  
  /**
   * Async task class for matching lines in a document.
   */
  private class SearchResultMatcherTask implements Callable<SearchResultEntry> {
    /**
     * The document to match in.
     */
    private final ScoreDoc _doc;
    /**
     * User provided query.
     */
    private final QueryContext _query;
    /**
     * Filter out overlapping results or not.
     */
    private final boolean _filterOverlapping;
    
    /**
     * The constructor.
     * 
     * @param query_ User provided query context.
     * @param doc_  The document to match in.
     * @param filterOverlapping_ filter out overlapping results or not.
     */
    SearchResultMatcherTask(QueryContext query_, ScoreDoc doc_,
      boolean filterOverlapping_) {
      _query = query_;
      _doc = doc_;
      _filterOverlapping = filterOverlapping_;
    }

    @Override
    public SearchResultEntry call() throws IOException {
      Context context = new Context(_query, _searcher, _doc.doc);

      FileInfo docsInfo = new FileInfo();
      docsInfo.id   = context.fileId;
      docsInfo.name = context.file.getName();
      docsInfo.path = context.file.getPath();
      
      ResultMatcher matcher = _matcherFactory.create(context);
      if (matcher == null) {
        _log.log(Level.SEVERE, "No line matcher found for this query! ");
        return null;
      }
      
      List<LineMatch> match = matcher.match();
      if (match.isEmpty()) {
        _log.log(Level.WARNING, "Empty match for {0} ({1})", 
          new Object[]{context.file.getPath(), context.fileId});
        return null;
      }
      
      if (_filterOverlapping) {
        match = filterOverlapping(match);
      }

      return new SearchResultEntry(match, docsInfo);
    }
    
    /**
     * @param range1_ A range that maybe contains the other.
     * @param range2_ The contained.
     * @return true if range1_ contains range2_, false otherwise.
     */
    private boolean isSecondInsideFirst(Range range1_, Range range2_) {
      return
        (range1_.startpos.line < range2_.startpos.line || (
          range1_.startpos.line == range2_.startpos.line &&
          range1_.startpos.column <= range2_.startpos.column)
        ) &&
        (range1_.endpos.line > range2_.endpos.line || (
          range1_.endpos.line == range2_.endpos.line &&
          range1_.endpos.column >= range2_.endpos.column)
        );
    }
    
    /**
     * Filters out the overlapping matches. The returned list may be the given
     * one or a new list. In both cases the given list will be modified.
     * 
     * @param matches_ a list of matches.
     * @return filtered matches.
     */
    private List<LineMatch> filterOverlapping(List<LineMatch> matches_) {
      if (matches_.size() <= 1) {
        return matches_;
      }
      
      // Sort by start position
      Collections.sort(matches_, new Comparator<LineMatch>() {
        @Override
        public int compare(LineMatch line1_, LineMatch line2_) {
          final Range range1 = line1_.range.range;
          final Range range2 = line2_.range.range;
          
          if (line1_.equals(line2_)) {
            return 0;
          }
          
          int res = Integer.compare(range1.startpos.line, range2.startpos.line);
          if (res != 0) {
            return res;
          }
          
          res = Integer.compare(range1.startpos.column, range2.startpos.column);
          if (res != 0) {
            return res;
          }
          
          if (isSecondInsideFirst(range1, range2)) {
            return -1;
          } else if (isSecondInsideFirst(range2, range1)) {
            return 1;
          } else {
            return 0;
          }
        }
      });
      
      final ArrayList<LineMatch> filteredMatches = new ArrayList<>(
        matches_.size());
      final Iterator<LineMatch> miter = matches_.iterator();
      
      LineMatch prev = miter.next();
      while (miter.hasNext()) {
        LineMatch curr = miter.next();
        
        if (!isSecondInsideFirst(prev.range.range, curr.range.range)) {
          filteredMatches.add(prev);
        }
        
        prev = curr;
      }
      filteredMatches.add(prev);

      return filteredMatches;
    }
  }

  /**
   * Opens the index database and initializes all members.
   *
   * @param options_ program options
   * @throws IOException
   */
  protected SearchAppCommon(CommonOptions options_) throws IOException {
    _options = options_;

    try {
      _indexReader = DirectoryReader.open(FSDirectory.open(
        new File(_options.indexDirPath), _options.createLockFactory()));
    } catch (IOException e) {
      _log.severe("Failed to open search index!");
      throw e;
    }

    _searcher = new IndexSearcher(_indexReader);

    Analyzer analyzer = new QueryAnalyzer();
    _textQueryParser = new QueryParser(Version.LUCENE_4_9,
      IndexFields.contentField, analyzer);
    _textQueryParser.setAllowLeadingWildcard(true);
    
    _executor = new ThreadPoolExecutor(10, Integer.MAX_VALUE, 10,
      TimeUnit.MINUTES, new SynchronousQueue<Runnable>());
    _matcherFactory = new MasterMatcherFactory();
  }

  /**
   * Closes the index and more... Actually it is a destructor.
   */
  @Override
  public void close() {
    _executor.shutdown();
    
    try {
      _matcherFactory.close();
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "Closing matcher factory failed!", ex);
    }
    
    if (_indexReader != null) {
      try {
        _indexReader.close();
      } catch (IOException e) {
        _log.log(Level.SEVERE, "Shit happened!!!", e);
      }
    }
  }

  /**
   * Does a document search.
   *
   * @param query_ Search query
   * @param filter_ Search filter
   * @param hitLimit_ Hit limit
   * @return Matching document ids
   * @throws IOException
   */
  protected TopDocs search(Query query_, Filter filter_, int hitLimit_)
    throws IOException {
    return _searcher.search(query_, filter_, hitLimit_);
  }

  /**
   * Does a document search with the default search limit.
   *
   * @param query_ Search query
   * @param filter_ Search filter
   * @return Matching document ids
   * @throws IOException
   */
  protected TopDocs search(Query query_, Filter filter_) throws IOException {
    return search(query_, filter_, DEFAULT_HIT_LIMIT);
  }

  /**
   * Compute search results for the query and its result docs.
   * 
   * @param query_ Search query context.
   * @param docs_ A user provided query.
   * @return result list.
   * @throws IOException 
   */
  protected List<SearchResultEntry> computeResultEntries(QueryContext query_,
    TopDocs docs_) throws IOException {

    ArrayList<Future<SearchResultEntry>> results = new ArrayList<>(
      docs_.scoreDocs.length);

    for (ScoreDoc doc : docs_.scoreDocs) {
      results.add(_executor.submit(new SearchResultMatcherTask(query_, doc,
        query_.getFilterOverlapping())));
    }
    
    ArrayList<SearchResultEntry> result = new ArrayList<>(
      docs_.scoreDocs.length);
    for (Future<SearchResultEntry> subres : results) {
      try {
        SearchResultEntry entry = subres.get();
        if (entry != null) {
          result.add(entry);
        }
      } catch (InterruptedException | ExecutionException ex) {
        _log.log(Level.WARNING, "Getting result failed!", ex);
      }
    }

    return result;
  }
  
  /**
   * Does a ranged document search.
   * 
   * @param query_ Search query
   * @param filter_ Search filter
   * @param startIndex_ Start index
   * @param endIndex_ End index (inclusive)
   * @return Matching document ids
   * @throws IOException
   */
  protected TopDocs rangedSearch(Query query_, Filter filter_, int startIndex_,
    int endIndex_) throws IOException {
    
    // Run the query
    TopDocs result = search(query_, filter_);

    if (result.scoreDocs.length <= startIndex_) {
      // Empty result
      return new TopDocs(result.totalHits, new ScoreDoc[0], 0);
    } else {
      // Real end index
      final int endIndex = Math.min(result.scoreDocs.length - 1, endIndex_);
      // Result array
      final ScoreDoc[] docs = new ScoreDoc[endIndex - startIndex_ + 1];

      for (int i = startIndex_; i <= endIndex; ++i) {
        docs[i - startIndex_] = result.scoreDocs[i];
      }

      return new TopDocs(result.totalHits, docs, result.getMaxScore());
    }
  }
}
