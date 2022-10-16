package cc.search.suggestion;

import cc.search.common.IndexFields;
import cc.search.common.SuggestionDatabase;
import cc.search.common.config.CommonOptions;
import java.io.IOException;
import java.io.File;
import java.io.FileOutputStream;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.core.WhitespaceAnalyzer;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.util.Version;
import org.apache.lucene.util.BytesRef;
import org.apache.lucene.search.suggest.InputIterator;
import org.apache.lucene.search.suggest.DocumentDictionary;
import org.apache.lucene.search.suggest.analyzing.AnalyzingInfixSuggester;
import org.apache.lucene.search.suggest.analyzing.FuzzySuggester;
import org.apache.lucene.store.OutputStreamDataOutput;
import org.apache.lucene.store.FSDirectory;

/**
 * Helper class for building the suggester databases.
 */
public final class DatabaseBuilder {
  
  /**
   * Logger.
   */
  private static final Logger _log = Logger.getLogger("GLOBAL_LOGGER");
  /**
   * An index reader for the main index database.
   */
  private final IndexReader _reader;
  /**
   * The program options.
   */
  private final CommonOptions _opts;

  /**
   * Constructs a DatabaseBuilder.
   *
   * @param reader_ an index reader for the main index database.
   * @param opts_ the program options.
   */
  public DatabaseBuilder(IndexReader reader_, CommonOptions opts_) {
    _reader = reader_;
    _opts = opts_;
  }

  /**
   * Builds all suggestion databases.
   */
  public void buildAll() {
    try {
      buildFileNameDb();
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Failed to create file name suggestions!", ex);
    }

    try {
      buildSymbolDb();
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Failed to create symbol suggestions!", ex);
    }
  }

  /**
   * Builds the suggester database for the FileName database.
   *
   * @throws IOException on any error.
   */
  private void buildFileNameDb() throws IOException {
    final File db = SuggestionDatabase.FileName.getDatabase(_opts, true);
    
    try (OutputStreamDataOutput out = new OutputStreamDataOutput(
      new FileOutputStream(db))) {

      final DocumentDictionary ddict = new DocumentDictionary(_reader,
        IndexFields.fileNameField, IndexFields.boostValue);

      final FuzzySuggester suggester = new FuzzySuggester(
        new WhitespaceAnalyzer(Version.LUCENE_4_9));

      suggester.build(ddict.getEntryIterator());
      
      if (!suggester.store(out)) {
        _log.log(Level.WARNING, "Failed to write out database to " +
          db.getAbsolutePath() + ". It's normal if the database is empty.");
      }
    }
  }

  /**
   * Builds the suggester database for the Symbol database.
   *
   * @throws IOException on any error.
   */
  private void buildSymbolDb() throws IOException {
    final File db = SuggestionDatabase.Symbol.getDatabase(_opts, true);

    try (AnalyzingInfixSuggester suggester = new AnalyzingInfixSuggester(
      Version.LUCENE_4_9, FSDirectory.open(db, _opts.createLockFactory()),
      new WhitespaceAnalyzer(Version.LUCENE_4_9))) {

      InputIterator iter = new UniqueInputIterator(
        new TagInputIterator(_reader)) {
        @Override
        protected void updateData(BytesRef item_, Data itemData_,
          InputIterator iter_) {
          if (itemData_.weight < iter_.weight()) {
            itemData_.weight = iter_.weight();
          }
        }
      };

      suggester.build(iter);
    }
  }
}

