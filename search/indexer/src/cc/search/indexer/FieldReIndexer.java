package cc.search.indexer;

import cc.parser.search.FieldValue;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;

/**
 * Indexer implementation for field re-indexing (see {@link
 *  cc.search.indexer.app.Indexer#addFieldValues(java.lang.String,
 *  java.util.Map)}).
 */
public class FieldReIndexer extends AbstractIndexer {
  /**
   * File database id.
   */
  private final String _fileId;
  /**
   * The fields that needs to be re-indexed.
   */
  private final Map<String, List<FieldValue>> _fieldValues;
  /**
   * An IndexReader.
   */
  private final IndexReader _indexReader;
  
  /**
   * 
   * @param fileId_ Value for {@link FieldReIndexer#_fileId}.
   * @param fieldValues_ Value for {@link FieldReIndexer#_fieldValues}.
   * @param indexWriter_ See {@link
   *  AbstractIndexer#AbstractIndexer(org.apache.lucene.index.IndexWriter)} for
   *  details.
   * @param indexReader_ An index reader for query for the document by db id.
   */
  public FieldReIndexer(String fileId_, Map<String, List<FieldValue>>
    fieldValues_, IndexWriter indexWriter_, IndexReader indexReader_) {
    
    super(indexWriter_);
    
    _fileId = fileId_;
    _fieldValues = fieldValues_;
    _indexReader = indexReader_;
  }
  
  @Override
  public Context createContext() throws IOException {
    Context ctx = new Context(_indexReader, _fileId);
    ctx.extraFields = _fieldValues;
    
    return ctx;
  }
}
