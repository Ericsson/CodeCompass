package cc.search.suggestion;

import cc.search.analysis.tags.Tag;
import cc.search.analysis.tags.Tags;
import cc.search.common.IndexFields;
import java.io.IOException;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.util.BytesRef;
import org.apache.lucene.search.suggest.InputIterator;

class TagInputIterator implements InputIterator {
  private final IndexReader _reader;
  private final DocumentIterator _docIter;
  private final Set<String> _fieldSet;
  private final Comparator<BytesRef> _comp;
  private Iterator<Tags.TagItem> _currTagIter = null;
  private Tag _currTag = null;

  public TagInputIterator(IndexReader reader_) throws IOException {
    _reader = reader_;
    _docIter = new DocumentIterator(reader_);
    _fieldSet = new HashSet<>();
    _fieldSet.add(IndexFields.tagsField);

    _comp = new Comparator<BytesRef>() {
      @Override
      public int compare(BytesRef o1_, BytesRef o2_) {
        return o1_.compareTo(o2_);
      }
    };
  }

  private boolean getNextTags() {
    while (_docIter.hasNext()) {
      try {
        BytesRef binVal = _reader.document(_docIter.next(), _fieldSet).
          getBinaryValue(IndexFields.tagsField);
        if (binVal == null) {
          continue;
        }
  
        _currTagIter = Tags.deserialize(binVal.bytes).getAllTags().iterator();
        if (!_currTagIter.hasNext()) {
          continue;
        }
  
        return true;
      } catch (IOException | ClassNotFoundException ex) {
        // just ignore
        continue;
      }
    }

    return false;
  }

  @Override
  public BytesRef next() {
    if (_currTagIter == null || !_currTagIter.hasNext()) {
      if (!getNextTags()) {
        return null;
      }
    }

    _currTag = _currTagIter.next().tag;
    return new BytesRef(_currTag.text.toLowerCase());
  }

  @Override
  public Comparator<BytesRef> getComparator() {
    return _comp;
  }

  @Override
  public Set<BytesRef> contexts() {
    return null;
  }

  @Override
  public boolean hasContexts() {
    return false;
  }

  @Override
  public boolean hasPayloads() {
    return true;
  }

  @Override
  public BytesRef payload() {
    return new BytesRef(_currTag.text);
  }

  @Override
  public long weight() {
    switch (_currTag.genericKind) {
      case Type:
        return 5;
      case Function:
        return 4;
      case Field:
        return 3;
      default:
        return 1;
    }
  }
}

