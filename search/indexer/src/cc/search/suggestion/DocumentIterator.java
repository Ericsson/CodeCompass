package cc.search.suggestion;

import java.io.IOException;
import java.util.Iterator;
import java.util.NoSuchElementException;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.MultiFields;
import org.apache.lucene.util.Bits;

/**
 * An iterator class for iterating documents (only doc id) from an IndexReader.
 */
class DocumentIterator implements Iterator<Integer> {
  /**
   * Next doc id.
   */
  private int _nextDocId = -1;
  /**
   * Max doc id (document count).
   */
  private final int _docCount;
  /**
   * Living documents.
   */
  private final Bits _liveDocs;

  /**
   * Constructs a new Iterator.
   *
   * @param reader_ the index reader.
   * @throws IOException on Lucene I/O errors.
   */
  public DocumentIterator(IndexReader reader_) throws IOException { 
    _docCount = reader_.maxDoc() - 1;
    _liveDocs = (reader_.leaves().size() > 0) ?
      MultiFields.getLiveDocs(reader_) : null;
    _nextDocId = getNextDocId();
  }

  /**
   * Returns the next document id (or an id >= _docCount if no more).
   *
   * @return the next id.
   */
  private int getNextDocId() {
    while (_nextDocId < _docCount) {
      ++_nextDocId;
      if (_liveDocs != null && !_liveDocs.get(_nextDocId)) {
        continue;
      }

      return _nextDocId;
    }

    return _docCount;
  }

  @Override
  public Integer next() throws NoSuchElementException {
    if (hasNext()) {
      Integer doc = _nextDocId;
      _nextDocId = getNextDocId();
      return doc;
    }

    throw new NoSuchElementException();
  }

  @Override
  public boolean hasNext() {
    return _nextDocId < _docCount;
  }

  @Override
  public void remove() throws UnsupportedOperationException {
    throw new UnsupportedOperationException("Not implemented!");
  }
}

