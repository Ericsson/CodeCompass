package cc.search.indexer.app;

import cc.parser.search.FieldValue;
import cc.parser.search.IndexerService;
import cc.search.analysis.SourceAnalyzer;
import cc.search.analysis.tags.TagGeneratorManager;
import cc.search.common.FileLoggerInitializer;
import cc.search.common.ipc.IPCProcessor;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;
import cc.search.indexer.FieldReIndexer;
import cc.search.indexer.FileIndexer;
import cc.search.indexer.IndexerTask;
import cc.search.suggestion.DatabaseBuilder;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.DirectoryReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.IndexWriterConfig;
import org.apache.lucene.index.IndexWriterConfig.OpenMode;
import org.apache.lucene.index.ReaderManager;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.util.Version;

/**
 * Creates or updates an index database.
 */
public class Indexer implements AutoCloseable, IndexerService.Iface {
  /**
   * Logger.
   */
  private static final Logger _log = Logger.getLogger(Indexer.class.getName());
  /**
   * Command line options.
   */
  private final Options _options;
  /**
   * This is the index directory.
   */
  private final Directory _indexDir;
  /**
   * The index database.
   */
  private final IndexWriter _indexWriter;
  /**
   * Reader manager for concurrent read/write. 
   */
  private final ReaderManager _readerManager;
  /**
   * List of asyc indexer results.
   */
  private final List<Future<Boolean>> _indexers;
  /**
   * Executor for async tasks.
   */
  private final ExecutorService _executor;
  /**
   * IPC message processor.
   */
  private final IPCProcessor _processor;
  /**
   * This member is only of logging: counts how many documents modified with
   * {@link Indexer#addFieldValues(java.lang.String, java.util.Map) }.
   */
  private long _docModifiedCounter = 0;

  /**
   * @param options_ command line options
   * @throws IOException 
   */
  private Indexer(Options options_) throws IOException {
    _options = options_;

    FileLoggerInitializer addFileLogger = new FileLoggerInitializer(_options, _log);
    addFileLogger.run();

    try {
      _indexDir = FSDirectory.open(new File(_options.indexDirPath),
        _options.createLockFactory());
      Analyzer analyzer = new SourceAnalyzer();

      IndexWriterConfig iwc = new IndexWriterConfig(Version.LUCENE_4_9,analyzer);
      iwc.setRAMBufferSizeMB(1024.0);

      switch (_options.indexOpenMode) {
        case CREATE:
          iwc.setOpenMode(OpenMode.CREATE);
          break;
        default:
          iwc.setOpenMode(OpenMode.CREATE_OR_APPEND);
          break;
      }
      
      _indexWriter = new IndexWriter(_indexDir, iwc);
      _readerManager = new ReaderManager(_indexWriter, true);
    } catch (IOException e) {
      _log.log(Level.SEVERE, "Failed to open search index!", e);
      throw e;
    }
    
    TagGeneratorManager.init();
    _executor = Executors.newCachedThreadPool();
    _indexers = new ArrayList<>();
    _processor = new IPCProcessor(options_,
      new IndexerService.Processor<Indexer>(this));
  }

  @Override
  public void close() {
    _processor.close();
    _executor.shutdown();
    
    try {
      _readerManager.close();
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Error on closing index reader!", ex);
    }
    
    try {
      _indexWriter.close();
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Error on closing index writer!", ex);
    }
    
    try {
      _indexDir.close();
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Error on closing index!", ex);
    }
    
    TagGeneratorManager.destroy();
  }

  /**
   * Gets the results of the indexing tasks.
   * 
   * @return number of successfully indexed files
   */
  private int waitFileIndexers() {
    int counter = 0;
    
    for (Future<Boolean> indexResult : _indexers) {
      try {
        if (indexResult.get()) {
          ++counter;
        }
      } catch (InterruptedException | ExecutionException ex) {
        _log.log(Level.WARNING, "Failed to index a file!", ex);
      }
    }
    
    _indexers.clear();
    
    return counter;
  }

  /**
   * Entry point.
   * 
   * @param args command line parameters.
   */
  public static void main(String[] args) {
    Indexer indexer;

    try {
      indexer = new Indexer(new Options(args));
    } catch (UnknownArgumentException | InvalidValueException | IOException e) {
      _log.log(Level.SEVERE, "Fatal error!", e);
      System.out.println(Options.getUsage());
      System.exit(-1);
      return;
    }

    try {
      _log.log(Level.INFO, "Indexer started! Mode: {0}",
        indexer._options.indexOpenMode.name());
      
      indexer._processor.serve();
        
      final int indexedFileCount = indexer.waitFileIndexers();
      _log.log(Level.INFO, "Indexed {0} file(s)", indexedFileCount);
      _log.log(Level.INFO, "Modified {0} file(s)", indexer._docModifiedCounter);
    } finally {
      indexer.close();
    }
    
    _log.log(Level.INFO, "Indexer finished!");
  }
  
  @Override
  public void stop() {
    _processor.stopServe();
  }

  @Override
  public void indexFile(String fileId_, String filePath_, String mimeType_) {
    _log.log(Level.FINEST, "Adding file {0} to index.", filePath_);
    
    try {
      _indexers.add(_executor.submit(new IndexerTask(
        new FileIndexer(filePath_, fileId_, mimeType_, _indexWriter))));
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "An unknown exception caught!", ex);
    }
  }

  @Override
  public void addFieldValues(String fileId_,
    Map<String, List<FieldValue>> fields_) throws org.apache.thrift.TException {
    
    _log.log(Level.FINEST, "Start adding extra values to file {0}", fileId_);
    try {
      _readerManager.maybeRefreshBlocking();
      
      final DirectoryReader reader = _readerManager.acquire();
      try {
        FieldReIndexer reindexer = new FieldReIndexer(fileId_, fields_,
          _indexWriter, reader);
        // We index this in the current thread to avoid collisions.
        if (reindexer.index()) {
          //_log.log(Level.FINEST, "Adding extra values ({1}) to file {0} " +
          //  "successful!", new Object[] { fileId_, fields_ });
          ++_docModifiedCounter;
        } else {
          _log.log(Level.FINEST, "Adding extra values to file {0} failed!",
            fileId_);
        }
      } finally {
        _readerManager.release(reader);
      }
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Adding extra values to file {0} failed with " +
        "exception!", ex);
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "An unknown exception caught!", ex);
    }
  }

  @Override
  public void buildSuggestions() {
    _log.log(Level.FINEST, "Start building suggestion databases");

    try {
      _readerManager.maybeRefreshBlocking();

      final DirectoryReader reader = _readerManager.acquire();
      try {
        final DatabaseBuilder builder = new DatabaseBuilder(reader, _options);
        builder.buildAll();
      } finally {
        _readerManager.release(reader);
      }
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Failed to build suggestion databases", ex);
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "An unknown exception caught!", ex);
    }
  }

  @Override
  public Map<String,String> getStatistics() {
    final HashMap<String, String> res = new HashMap<>();

    try {
      _readerManager.maybeRefreshBlocking();
      
      final DirectoryReader reader = _readerManager.acquire();
      try {
        res.put("Documents in index", Integer.toString(reader.numDocs()));
      } finally {
        _readerManager.release(reader);
      }
    } catch (IOException ex) {
      _log.log(Level.SEVERE, "Adding extra values to file {0} failed with " +
        "exception!", ex);
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "An unknown exception caught!", ex);
    }

    return res;
  }
}
