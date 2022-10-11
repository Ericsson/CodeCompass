package cc.search.service.app.service;

import cc.search.common.FileLoggerInitializer;
import cc.search.analysis.AdvancedTagQueryParser;
import cc.search.analysis.QueryAnalyzer;
import cc.search.analysis.log.LogQueryBuilder;
import cc.search.analysis.query.MatchCollector;
import cc.search.analysis.query.SimpleMatchCollector;
import cc.search.analysis.tags.Tag;
import cc.search.common.config.CommonOptions;
import cc.search.common.IndexFields;
import cc.search.common.SuggestionDatabase;
import cc.search.match.QueryContext;
import cc.search.service.app.SearchAppCommon;
import cc.search.suggestion.SuggestionHandler;
import cc.service.search.FileSearchResult;
import cc.service.core.InvalidId;
import cc.service.search.SearchResult;
import cc.service.search.SearchResultEntry;
import cc.service.search.SearchException;
import cc.service.search.SearchOptions;
import cc.service.search.SearchParams;
import cc.service.search.SearchSuggestionParams;
import cc.service.search.SearchSuggestions;
import cc.service.search.SearchService;
import cc.service.search.SearchType;
import java.io.IOException;
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryparser.classic.ParseException;
import org.apache.lucene.search.BooleanClause;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Filter;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.QueryWrapperFilter;
import org.apache.lucene.search.RegexpQuery;
import org.apache.lucene.search.TopDocs;
import org.apache.thrift.TException;

/**
 * Search service implementation.
 */
abstract class SearchHandler extends SearchAppCommon
  implements SearchService.Iface {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger(SearchHandler.class
    .getName());
  /**
   * Handler for suggestion requests.
   */
  private final SuggestionHandler _suggestHandler;
  /**
   * Query parser for advanced definition search.
   */
  protected final AdvancedTagQueryParser _advDefQueryParser;
  /**
   * Query builder for a log query.
   */
  protected final LogQueryBuilder _logQueryBuilder = new LogQueryBuilder();
  /**
   * Construct a filter for a search query.
   * 
   * @param params_ search parameters.
   * @return a filter or null on no filtering.
   */
  private Filter getFilterForSearch(SearchParams params_) {
    if (!params_.isSetFilter()) {
      return null;
    }
    
    BooleanQuery filterQuery = new BooleanQuery();
    
    final String dirFilter = params_.filter.dirFilter.trim();
    if (!dirFilter.isEmpty()) {
      filterQuery.add(new RegexpQuery(new Term(IndexFields.fileDirPathField,
        dirFilter.toLowerCase())), BooleanClause.Occur.MUST);
    }
    
    final String fnameFilter = params_.filter.fileFilter.trim();
    if (!fnameFilter.isEmpty()) {
      filterQuery.add(new RegexpQuery(new Term(IndexFields.fileNameField,
        fnameFilter.toLowerCase())), BooleanClause.Occur.MUST);
    }
    
    if (filterQuery.clauses().isEmpty()) {
      return null;
    }
    
    return new QueryWrapperFilter(filterQuery);
  }
  
  /**
   * Does a full text search.
   * 
   * @param context_ query context.
   * @param params_ search parameters.
   * @return search query result.
   * @throws ParseException
   * @throws IOException 
   */
  private SearchResult runSearch(QueryContext context_, SearchParams params_)
    throws ParseException, IOException {
    _log.info("Running text query...");
    _log.finer(params_.toString());

    Date start = new Date();
    
    final Filter filter = getFilterForSearch(params_);
    TopDocs docs;
    
    if (params_.isSetRange()) {
      docs = rangedSearch(context_.get(), filter, (int) params_.range.start,
        (int) (params_.range.start + params_.range.maxSize - 1));
    } else {
      docs = search(context_.get(), filter);
    }
    
    _log.log(Level.INFO, "Got {1} doc(s) in {0} total milliseconds",
        new Object[] {
          new Date().getTime() - start.getTime(),
          docs.scoreDocs.length
        });

    start = new Date();
    List<SearchResultEntry> entries = computeResultEntries(context_, docs);
    
    _log.log(Level.INFO, "Got {1} result(s) in {0} total milliseconds",
        new Object[] {
          new Date().getTime() - start.getTime(),
          entries.size()
        });
    
    return new SearchResult(
      Math.min(docs.totalHits, DEFAULT_HIT_LIMIT), entries);
  }
  
  /**
   * @param options_ command line options.
   * @throws IOException 
   */
  public SearchHandler(CommonOptions options_) throws IOException {
    super(options_);

    FileLoggerInitializer addFileLogger = new FileLoggerInitializer(options_, _log);
    addFileLogger.run();
    
    Analyzer analyzer = new QueryAnalyzer();
    
    _advDefQueryParser = new AdvancedTagQueryParser(analyzer);
    _advDefQueryParser.setAllowLeadingWildcard(true);

    _suggestHandler = new SuggestionHandler();
    _suggestHandler.loadDatabases(options_);
  }

  @Override
  public SearchResult search(SearchParams params_) throws TException {
    try {
      QueryContext qcontext = new QueryContext();
      
      // "Free text" search
      if ((params_.options & SearchOptions.SearchInSource.getValue()) != 0) {
        qcontext.add(QueryContext.QueryType.Text,
          _textQueryParser.parse(params_.query));
      }
      // Definition search
      // Advanced search
      if ((params_.options & SearchOptions.SearchInDefs.getValue()) != 0) {
        final Query query = _advDefQueryParser.parse(params_.query);
        final Set<Tag.Kind> kinds = _advDefQueryParser.getParsedKinds();
        
        qcontext.add(QueryContext.QueryType.Tag, query, kinds);
        qcontext.setFilterOverlapping(true);
      }
      // Log search
      if ((params_.options & SearchOptions.FindLogText.getValue()) != 0) {
        final MatchCollector collector = new SimpleMatchCollector(0);
        qcontext.add(QueryContext.QueryType.Log, _logQueryBuilder.build(
          params_.query, collector), collector);
      }
      
      if (qcontext.isEmpty()) {
        // empty search
        return new SearchResult(0, null);
      } else {
        return runSearch(qcontext, params_);
      }
    } catch (IllegalArgumentException | ParseException | IOException ex) {
      // Sometimes the query parser throws an IllegalArgumentException instead
      // of a ParseException (maybe it will be fixed in a new Lucene release).
      _log.log(Level.SEVERE, "Search failed!", ex);
      SearchException exc = new SearchException();
      exc.message = ex.getMessage();
      throw exc;
    }
  }

  @Override
  public FileSearchResult searchFile(SearchParams params_) throws TException {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public List<SearchType> getSearchTypes() throws InvalidId, TException {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public SearchSuggestions suggest(SearchSuggestionParams params_)
    throws TException {
    SearchSuggestions result = new SearchSuggestions();
    result.setTag(params_.getTag());

    SuggestionDatabase db = null;
    if ((params_.options & SearchOptions.SearchForFileName.getValue()) != 0) {
      db = SuggestionDatabase.FileName;
    } else if ((params_.options & SearchOptions.SearchInDefs.getValue()) != 0){
      db = SuggestionDatabase.Symbol;
    } else {
      SearchException exc = new SearchException();
      exc.message = "Can't make suggestions for this type!";
      throw exc;
    }

    result.setResults(_suggestHandler.suggest(db,
      params_.userInput.toLowerCase(), params_.limit));

    return result;
  }

  @Override
  public void close() {
    _suggestHandler.close();

    super.close();
  }
}
