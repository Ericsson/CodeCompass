package cc.search.indexer;

import com.j256.simplemagic.ContentInfo;
import com.j256.simplemagic.ContentInfoUtil;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.index.IndexWriter;

/**
 * Class for indexing a file.
 */
public class FileIndexer extends AbstractIndexer {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * File path to index.
   */
  private final String _filePath;
  /**
   * The database id of the file.
   */
  private final String _fileId;
  /**
   * The mime type of the file.
   */
  private final String _fileMimeType;
  

  /**
   * @param file_ file to index
   * @param fileId_ database id of the file
   * @param mimeType_ mime type of the file.
   * @param indexWriter_ index database
   */
  public FileIndexer(String file_, String fileId_, String mimeType_,
    IndexWriter indexWriter_) {
    super(indexWriter_);
    
    _filePath = file_;
    _fileId = fileId_;
    _fileMimeType = mimeType_;
  }
  
  @Override
  public Context createContext() {
    File file = new File(_filePath);
    if (!file.canRead()) {
      _log.log(Level.SEVERE, "Couldn''t read {0}! Skipping!", file.getPath());
      return null;
    }

    if (file.isFile()) {
      try {
        String mimeType = _fileMimeType;
        if (mimeType.equals("text/plain")) {
          String fileType = tryDetectMimeType(new File(_filePath));
          if (fileType != null && !fileType.equals(mimeType)) {
            mimeType = fileType;
            _log.log(Level.FINER, "Got a better file type for {0}: {1}",
                new Object[]{_filePath, fileType});
          }
        }
        
        return new Context(_fileId, file, mimeType);
      } catch (FileNotFoundException e) {
        _log.log(Level.SEVERE, "File not found: {0}! Skipping!",file.getPath());
        return null;
      } catch (IOException e) {
        _log.log(Level.SEVERE, "Shit happened!", e);
        return null;
      }
    }
    else {
      _log.log(Level.SEVERE, "{0} is not a file! Skipping!", file.getPath());
      return null;
    }
  }
  
  /**
   * Tries to detect the mime-type of a given file.
   * 
   * NOTE:
   * First I tried Files.probeContentType(Path) but it causes a SEGFAULT on some
   * machines in the JVM.
   * 
   * @param file_ a file.
   * @return a mime type or null failure.
   */
  private static String tryDetectMimeType(File file_) {
    try {
      final ContentInfoUtil util = new ContentInfoUtil();
      
      ContentInfo cinfo = util.findMatch(file_);
      if (cinfo == null) {
        cinfo = ContentInfoUtil.findExtensionMatch(file_.getName());
      }
      
      if (cinfo != null) {
        return cinfo.getMimeType();
      }
    } catch (IllegalStateException | IOException ex) {
      _log.log(Level.WARNING, "Something bad happened during the mime-type " +
        "detection!", ex);
    }
    
    return null;
  }
}
