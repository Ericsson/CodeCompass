package cc.search.match;

import cc.search.analysis.LineInformations;
import cc.search.common.IndexFields;
import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import org.apache.lucene.document.Document;
import org.apache.lucene.search.IndexSearcher;

/**
 * Matching context.
 */
public class Context {
  /**
   * Matching query context.
   */
  public final QueryContext query;
  /**
   * Document database id.
   */
  public final int documentId;
  /**
   * Lucene document.
   */
  public final Document document;
  /**
   * A shared index searcher.
   */
  public final IndexSearcher searcher;
  /**
   * Original file content.
   */
  public final String originalContent;
  /**
   * Line informations based on file content.
   */
  public final LineInformations lineInfos;
  /**
   * File object for getting file path.
   */
  public final File file;
  /**
   * Thrift file id.
   */
  public final String fileId;
  /**
   * @param query_ query context.
   * @param searcher_ a shared index searcher.
   * @param docId_ document database id.
   * @throws IOException 
   */
  public Context(QueryContext query_, IndexSearcher searcher_, int docId_)
    throws IOException {
    query = query_;
    documentId = docId_;
    document = searcher_.getIndexReader().document(docId_);
    searcher = searcher_;
    originalContent = document.get(IndexFields.contentField);
    file = new File(document.get(IndexFields.filePathField));
    fileId = document.get(IndexFields.fileDbIdField);
    
    lineInfos = LineInformations.fromReader(new StringReader(originalContent));
  }
}
