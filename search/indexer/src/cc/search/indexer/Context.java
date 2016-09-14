package cc.search.indexer;

import cc.parser.search.FieldValue;
import cc.search.analysis.LineInformations;
import cc.search.common.IndexFields;
import cc.search.indexer.util.IOHelper;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.util.List;
import java.util.Map;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexReader;

/**
 * Analysis context.
 */
public final class Context {
  /**
   * The document.
   */
  public final Document document;
  /**
   * Additional fields or null.
   */
  public Map<String, List<FieldValue>> extraFields = null;
  /**
   * Line informations.
   */
  public final LineInformations lineInfos;
  
  /**
   * @return file db id.
   */
  public String getFileId() {
    return document.get(IndexFields.fileDbIdField);
  }
  
  /**
   * @return full absolute path.
   */
  public String getFileFullPath() {
    return document.get(IndexFields.filePathField);
  }
  
  /**
   * Builds a context by reading the given file.
   * 
   * @param file_ file.
   * @param fileId_ file database id.
   * @param fileMimeType_  mime type.
   * @throws FileNotFoundException
   * @throws IOException 
   */
  public Context(String fileId_, File file_, String fileMimeType_)
    throws FileNotFoundException, IOException {
    // Read content from a file stream
    try (FileInputStream stream = new FileInputStream(file_)) {
      String fileContent = IOHelper.readFullContent(
        IOHelper.getReaderForInput(stream));
      
      // Get line informations
      try (Reader reader = new StringReader(fileContent)) {
        lineInfos = LineInformations.fromReader(reader);
        
        document = AbstractIndexer.createDocumentForFile(fileId_, file_,
          fileContent, fileMimeType_);
      }
    }
  }
  
  /**
   * Opens an existing document for update.
   * 
   * @param reader_ an index reader.
   * @param fileId_ a file id.
   * @throws IOException 
   */
  public Context(IndexReader reader_, String fileId_) throws IOException {
    document = AbstractIndexer.loadDocumentWithMetadata(reader_, fileId_);
    
    // Get line informations
    try (Reader reader = new StringReader(document.get(
      IndexFields.contentField))) {
      lineInfos = LineInformations.fromReader(reader);
    }
  }
}
