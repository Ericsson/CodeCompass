package cc.search.indexer;

import cc.parser.search.FieldValue;
import cc.parser.search.search_indexerConstants;
import cc.search.analysis.Location;
import cc.search.analysis.tags.Tag;
import cc.search.analysis.tags.TagGenerator;
import cc.search.analysis.tags.TagGeneratorManager;
import cc.search.analysis.tags.TagStream;
import cc.search.analysis.tags.Tags;
import cc.search.common.IndexFields;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.document.FieldType;
import org.apache.lucene.document.NumericDocValuesField;
import org.apache.lucene.document.StoredField;
import org.apache.lucene.document.StringField;
import org.apache.lucene.document.TextField;
import org.apache.lucene.index.FieldInfo;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.IndexWriterConfig.OpenMode;
import org.apache.lucene.index.IndexableField;
import org.apache.lucene.index.Term;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.TermQuery;
import org.apache.lucene.search.TopDocs;
import org.apache.lucene.util.BytesRef;

/**
 * Indexer.
 */
abstract public class AbstractIndexer {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger(AbstractIndexer
    .class.getName());
  /**
   * Field type for storing tags.
   */
  protected final static FieldType _tagFieldType;
  /**
   * Field type for storing content.
   */
  protected final static FieldType _contentFieldType;
  /**
   * An index database.
   */
  protected final IndexWriter _indexWriter;
  
  static {
    _tagFieldType = new FieldType();
    _tagFieldType.setIndexed(true);
    _tagFieldType.setTokenized(true);
    _tagFieldType.setStored(false);
    _tagFieldType.setStoreTermVectors(true);
    _tagFieldType.setStoreTermVectorPositions(true);
    _tagFieldType.setStoreTermVectorOffsets(true);    
    _tagFieldType.freeze();
    
    _contentFieldType = new FieldType();
    _contentFieldType.setIndexOptions(
      FieldInfo.IndexOptions.DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    _contentFieldType.setIndexed(true);
    _contentFieldType.setTokenized(true);
    _contentFieldType.setStored(true);
    _contentFieldType.setStoreTermVectors(true);
    _contentFieldType.setStoreTermVectorPositions(true);
    _contentFieldType.setStoreTermVectorOffsets(true);
    _contentFieldType.freeze();
  }
  
  /**
   * @param indexWriter_ an index database.
   */
  AbstractIndexer(IndexWriter indexWriter_) {
    _indexWriter = indexWriter_;
  }
  
  public abstract Context createContext() throws IOException;
  
  /**
   * Indexes a file and adds it to the database.
   * 
   * @return true on success, false on fail.
   */
  public boolean index() {
    try {
      final Context ctx = createContext();
      final Tags tags = generateTagsForContext(ctx);
      
      if (ctx.extraFields != null) {
        Iterator<String> extraFieldIter = ctx.extraFields.keySet().iterator();
        
        while (extraFieldIter.hasNext()) {
          String extraField = extraFieldIter.next();
          switch (extraField) {
            case search_indexerConstants.FIELD_DEFINITIONS:
              // Extra definitions
              appendTagsFromFieldValues(extraField, ctx, tags);
              break;
            case search_indexerConstants.FIELD_PARSE_STATUS: {
              // Extra boot by parse status
                long extraBoost = 0;
                long origBoost = 1;

                final List<FieldValue> values =ctx.extraFields.get(extraField);
                if (values == null || values.size() != 1) {
                  _log.log(Level.SEVERE, "{0} needs exactly on field value!",
                    extraField);
                  break;
                }

                switch (values.get(0).value) {
                  case search_indexerConstants.PSTATUS_PARSED:
                  case search_indexerConstants.PSTATUS_PART_PARSED:
                    extraBoost = 1;
                    break;
                  case search_indexerConstants.PSTATUS_NOT_PARSED:
                    extraBoost = 0;
                    break;
                  default:
                    _log.log(Level.SEVERE, "Bad field value {1} for {0}!",
                      new Object[] { extraField, values.get(0).value });
                    break;
                }

                if (extraBoost <= 0) {
                  break;
                }

                IndexableField boostFld = ctx.document.getField(
                  IndexFields.boostValue);
                if (boostFld != null) {
                  origBoost = boostFld.numericValue().longValue();
                }

                _log.log(Level.FINE, "Set boost from {0} to {1}.",
                  new Object[] { origBoost, origBoost + extraBoost });

                ctx.document.removeFields(IndexFields.boostValue);
                ctx.document.add(new NumericDocValuesField(
                  IndexFields.boostValue, origBoost + extraBoost));
              }
              break;
            default:
              _log.log(Level.WARNING, "Skipping filed: {0}", extraField);
              break;
          }
        }
      }
      
      replaceTagsInDocument(ctx.document, tags);
      insertDocumentToIndex(ctx);
      
      return true;
    } catch (IOException ex) {
      _log.log(Level.INFO, "Failed to (re)index file: {0}", ex.getMessage());
      return false;
    }
  }
  
  /**
   * Loads a document by database id and sets its field with correct meta data.
   * Actually it makes a copy from the original document.
   * 
   * NOTE: tags streams are not copied only the IndexFields.tagsField.
   * 
   * @param reader_ an IndexReader.
   * @param fileId_ the file`s database id.
   * @return a Document that looks like the original one (except the tag
   *  streams).
   * @throws IOException 
   */
  static Document loadDocumentWithMetadata(IndexReader reader_,
    String fileId_) throws IOException {
    final IndexSearcher searcher = new IndexSearcher(reader_);
    final TermQuery query = new TermQuery(new Term(IndexFields.fileDbIdField,
      fileId_));
    final TopDocs docs = searcher.search(query, 1);
    
    if (docs.scoreDocs.length != 1) {
      throw new IOException("Document with id: '" + fileId_ + "' not found! ");
    }
    
    final Document oldDoc = reader_.document(docs.scoreDocs[0].doc);
    final File origFile = new File(oldDoc.get(IndexFields.filePathField));
    final String origContent = oldDoc.get(IndexFields.contentField);
    final String origContentMime = oldDoc.get(IndexFields.mimeTypeField);
    final BytesRef origTagsBin = oldDoc.getBinaryValue(IndexFields.tagsField);
    
    final Document doc = createDocumentForFile(fileId_, origFile, origContent,
      origContentMime);
    
    if (origTagsBin != null) {
      doc.add(new StoredField(IndexFields.tagsField, origTagsBin));
    }
    
    return doc;
  }
  
  /**
   * Creates a document for a given file and sets its common fields.
   * 
   * @param fileId_ file database id.
   * @param file_file object for file path.
   * @param fileContent_ file content as string.
   * @param fileMimeType_ file content mime type.
   * @return a new document.
   */
  static Document createDocumentForFile(String fileId_, File file_,
    String fileContent_, String fileMimeType_) {
    Document doc = new Document();
    
    // File path
    doc.add(new TextField(IndexFields.filePathField, file_.getAbsolutePath(),
      Field.Store.YES));
    // File name
    doc.add(new StringField(IndexFields.fileNameField,
      file_.getName().toLowerCase(), Field.Store.YES));
    // Directory path
    doc.add(new StringField(IndexFields.fileDirPathField,
      file_.getParentFile().getAbsolutePath().toLowerCase(), Field.Store.NO));
    // File id
    doc.add(new StringField(IndexFields.fileDbIdField, fileId_,
      Field.Store.YES));
    // Mime type
    doc.add(new StringField(IndexFields.mimeTypeField, fileMimeType_,
      Field.Store.YES));
    // Text content
    doc.add(new Field(IndexFields.contentField, fileContent_,
      _contentFieldType));
    
    if (isSourceFile(fileMimeType_)) {
      doc.add(new NumericDocValuesField(IndexFields.boostValue, 2L));
    } else {
      doc.add(new NumericDocValuesField(IndexFields.boostValue, 1L));
    }
    
    return doc;
  }
  
  /**
   * @param fileMimeType_ a file mime-type.
   * @return ture if the file is a source file by its mime-type, false
   *         otherwise.
   */
  private static boolean isSourceFile(String fileMimeType_) {
    switch (fileMimeType_) {
      case "text/x-c":
      case "text/x-c++":
      case "text/x-php":
      case "text/x-shellscript":
      case "text/x-awk":
      case "text/x-gawk":
      case "text/x-nawk":
      case "text/x-java":
      case "text/x-java-source":
      case "text/x-m4":
      case "text/x-makefile":
      case "text/x-perl":
      case "text/x-python":
      case "text/x-ruby":
      case "text/x-tcl":
      case "text/x-javascript":
      case "application/javascript":
      case "application/x-shellscript":
        return true;
      default:
        return false;
    }
  }
  
  /**
   * Generates tags for the given document or loads a previous version if it
   * exists.
   * 
   * @param context_ indexer context.
   * @return a tags container.
   * @throws IOException 
   */
  private static Tags generateTagsForContext(Context context_)
    throws IOException {
    BytesRef tagsBin = context_.document.getBinaryValue(IndexFields.tagsField);
    if (tagsBin == null) {
      TagGenerator generator = TagGeneratorManager.get().getGenerator();
      try {
        Tags tags = new Tags();
        generator.generate(tags, context_);

        return tags;
      } finally {
        TagGeneratorManager.get().releaseGenerator(generator);
      }
    } else {
      try {
        return Tags.deserialize(tagsBin.bytes);
      } catch (ClassNotFoundException ex) {
        // impossible
        throw new IOException(ex);
      }
    }
  }
  
  /**
   * Adds tags the from user provided extra field map (if any).
   * 
   * @param field a document field.
   * @param context_ indexer context.
   * @param tags_ tag container.
   */
  private static void appendTagsFromFieldValues(String field_,
    Context context_, Tags tags_) {
    if (context_.extraFields == null) {
      return;
    }
    
    final List<FieldValue> values = context_.extraFields.get(field_);
    if (values == null) {
      return;
    }
    
    for (FieldValue value : values) {
      try {
        // Only single line tags supported
        if (value.location.startLine != value.location.endLine) {
          // Only single line names are supported
          value.location.endLine = value.location.startLine;
          value.location.endColumn = context_.lineInfos.getLineContent(
            (int) value.location.startLine).length() + 1; 
        }

        // Workaround for bad "end-of-line" position
        value.location.startColumn = Math.min(value.location.startColumn,
          value.location.endColumn);

        Tag.Kind kind;
        try {
          kind = Tag.Kind.valueOf(value.context);
        } catch (IllegalArgumentException ex) {
          _log.log(Level.SEVERE, "Bad kind!", ex);
          continue;
        }

        Location loc;
        try {
          // I assume that we got a location with exclusive end (as it is in
          // the thrift API documentation), so let`s convert it to inclusive.
          loc = new Location(
            (int) value.location.startLine,
            (int) value.location.startColumn,
            (int) value.location.endColumn - 1);
        } catch (IllegalArgumentException ex) {
          _log.log(Level.FINE, "Skipping tag: ''{0}'' due to its wrong " +
            "location: {1}", new Object[]{value.value,
              value.location.toString()});
          continue;
        }

        // Convert field to a tag
        final Tag tag = new Tag(loc, value.value, value.context, kind);

        // Calculate start offset
        final int offset = context_.lineInfos.locationToStartOffset(
          tag.location);

        tags_.add(tag, offset);
      } catch (IndexOutOfBoundsException ex) {
        _log.log(Level.FINE, "Possibly bad line or column number!", ex);
      }
    }
  }
  
  /**
   * Adds or replaces the tag token streams in the document.
   * 
   * @param doc_ a document.
   * @param tags_ tags to add.
   * @throws IOException 
   */
  private static void replaceTagsInDocument(Document doc_, Tags tags_)
    throws IOException {
    // Add/replace field for definition search
    doc_.removeFields(IndexFields.definitionsField); 
    doc_.add(new Field(IndexFields.definitionsField,
      new TagStream(tags_, new TagStream.NullFilter()), _tagFieldType));
    
    // Advanced search fields (kinds#1)
    for (Tag.Kind kind : Tag.Kind.values()) {
      String kindFieldName = IndexFields.getFieldNameForTagKind(kind);
      doc_.removeFields(kindFieldName);
      doc_.add(new Field(kindFieldName,
        new TagStream(tags_, new TagStream.KindFilter(kind)), _tagFieldType));
    }

    // Advanced search fields (kinds#2)
    final Collection<String> kinds = tags_.calculateOriginalKindSet();
    final StringBuilder kindsTextBuilder = new StringBuilder(1000);
    for (final String kind : kinds) {
      kindsTextBuilder.append(kind).append("\n");
    }
    doc_.removeFields(IndexFields.tagKindsField);
    doc_.add(new TextField(IndexFields.tagKindsField,
      kindsTextBuilder.toString(), TextField.Store.YES));

    // Serialize tags
    doc_.removeFields(IndexFields.tagsField);
    doc_.add(new StoredField(IndexFields.tagsField, tags_.serialize()));
  }
  
  /**
   * Adds the given document to the document index or updates an old version.
   * 
   * @param context_ the context.
   * @throws IOException 
   */
  protected void insertDocumentToIndex(Context context_) throws IOException {
    if (_indexWriter.getConfig().getOpenMode() == OpenMode.CREATE) {
      _indexWriter.addDocument(context_.document);
    } else {
      _indexWriter.updateDocument(new Term(IndexFields.fileDbIdField,
        context_.getFileId()), context_.document);
    }
  }
}

