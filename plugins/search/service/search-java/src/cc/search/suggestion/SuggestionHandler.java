package cc.search.suggestion;

import cc.search.common.SuggestionDatabase;
import cc.search.common.config.CommonOptions;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.lang.AutoCloseable;
import org.apache.lucene.analysis.core.WhitespaceAnalyzer;
import org.apache.lucene.util.Version;
import org.apache.lucene.search.suggest.Lookup;
import org.apache.lucene.search.suggest.analyzing.AnalyzingInfixSuggester;
import org.apache.lucene.search.suggest.analyzing.FuzzySuggester;
import org.apache.lucene.store.InputStreamDataInput;
import org.apache.lucene.store.FSDirectory;

/**
 * Helper class for handling suggest() requests.
 */
public final class SuggestionHandler implements AutoCloseable {
  /**
   * Logger.
   */
  private static final Logger _log = Logger.getLogger("GLOBAL_LOGGER");
  /**
   * Filename suggester.
   */
  private final FuzzySuggester _fileNameSuggester = new FuzzySuggester(
    new WhitespaceAnalyzer(Version.LUCENE_4_9));
  /**
   * Symbol suggester.
   */
  private AnalyzingInfixSuggester _symbolSuggester = null;
  
  /**
   * Loads the suggestion databases.
   *
   * @param opts_ the program options.
   */
  public void loadDatabases(CommonOptions opts_) {
    try {
      loadFileNameSuggester(opts_);
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Failed to load the file name suggestion " +
        "database!", ex);
    }

    try {
      _symbolSuggester = new AnalyzingInfixSuggester(
        Version.LUCENE_4_9,
        FSDirectory.open(SuggestionDatabase.Symbol.getDatabase(opts_),
          opts_.createLockFactory()),
        new WhitespaceAnalyzer(Version.LUCENE_4_9));
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Failed to load the symbol suggestion database!",
        ex);
    }
  }

  /**
   * Suggests some search text based on user input.
   *
   * @param db_ which database should be used.
   * @param userText_ the user input.
   * @param limit_ the maximum number of results.
   * @return a list of result texts.
   */
  public List<String> suggest(SuggestionDatabase db_, String userText_,
    long limit_) {
    final ArrayList<String> result = new ArrayList<>(
      (int) Math.min(100L, limit_));

    try {
      List<Lookup.LookupResult> lrs = null;
      switch (db_) {
        case FileName:
          lrs = _fileNameSuggester != null ?
            _fileNameSuggester.lookup(userText_, false, (int) limit_) :
            null;
          break;
        case Symbol:
          lrs = _symbolSuggester != null ?
            _symbolSuggester.lookup(userText_, (int) limit_, false, false) :
            null;
          break;
      }

      if (lrs != null) {
        for (Lookup.LookupResult lkr : lrs) {
          if (lkr.payload != null) {
            result.add(lkr.payload.utf8ToString());
          } else {
            result.add(lkr.key.toString());
          }
        }
      }
    } catch (IllegalStateException | IOException ex) {
      _log.log(Level.SEVERE, "Lookup failed!", ex);
    }

    return result;
  }

  /**
   * Loads the filename suggester database.
   *
   * @param opts_ the program options.
   * @throws IOException on any I/O error.
   */
  private void loadFileNameSuggester(CommonOptions opts_) throws IOException {
    final File db = SuggestionDatabase.FileName.getDatabase(opts_);
    
    try (InputStreamDataInput in = new InputStreamDataInput(
      new FileInputStream(db))) {
      _fileNameSuggester.load(in);
    }
  }

  @Override
  public void close() {
    if (_symbolSuggester != null) {
      try {
        _symbolSuggester.close();
      } catch (IOException ex) {
        // Don't care...
      }
    }
  }
}

