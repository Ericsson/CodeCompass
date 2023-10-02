package cc.search.service.app.query;

import cc.search.service.app.SearchAppCommon;
import cc.search.common.FileLoggerInitializer;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;
import cc.search.match.QueryContext;
import cc.service.search.SearchResultEntry;
import org.apache.lucene.queryparser.classic.ParseException;
import org.apache.lucene.search.Query;

import java.io.IOException;
import java.util.Date;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.search.TopDocs;

/**
 * Application for testing the search queries.
 */
public class QueryApp extends SearchAppCommon {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * Application options.
   */
  private final QueryAppOptions _appOptions;
  
  /**
   * @param options_ command line options.
   * @throws IOException 
   */
  private QueryApp(QueryAppOptions options_) throws IOException {
    super(options_);
    _appOptions = options_;
    FileLoggerInitializer.addFileOutput(options_, _log, "query");
  }

  /**
   * Runs a query and prints the result to the standard output.
   * 
   * @throws IOException 
   */
  public void queryAndPrintResults() throws IOException {
    try {
      Query query = _textQueryParser.parse(_appOptions.queryString);

      _log.info("Running query...");
      Date start = new Date();

      TopDocs docs = search(query, null);

      Date qtime = new Date();
      _log.log(Level.INFO, "Got doc(s) in {0} total milliseconds",
        qtime.getTime() - start.getTime());

      QueryContext qcontext = new QueryContext();
      qcontext.add(QueryContext.QueryType.Text, query);
      List<SearchResultEntry> searchResultEntries =
        computeResultEntries(qcontext, docs);

      _log.log(Level.INFO, "Got result(s) in {0} total milliseconds",
        new Date().getTime() - qtime.getTime());
      
      for (SearchResultEntry entry : searchResultEntries) {
        System.out.println(entry.toString());
      }
      System.out.println();
    } catch (ParseException e) {
      _log.log(Level.SEVERE, "Failed to parse query!", e);
    }
  }

  public static void main(String[] args_)  {
    try (QueryApp app = new QueryApp(new QueryAppOptions(args_))) {
      app.queryAndPrintResults();
    } catch (UnknownArgumentException | InvalidValueException | IOException e) {
      _log.log(Level.SEVERE, "Fatal error!", e);
      System.out.println(QueryAppOptions.getUsage());
      System.exit(-1);
    }
  }
}
