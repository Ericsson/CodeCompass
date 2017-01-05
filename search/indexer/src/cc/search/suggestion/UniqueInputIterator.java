package cc.search.suggestion;

import java.io.IOException;
import java.util.Map;
import java.util.TreeMap;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Set;
import org.apache.lucene.util.BytesRef;
import org.apache.lucene.search.suggest.InputIterator;

/**
 * This input itarator collects the unique items from an another input
 * iterator.
 */
class UniqueInputIterator implements InputIterator {
  /**
   * Helper class for holding extra data for an item.
   */
  protected static class Data {
    /**
     * Contexts.
     */
    Set<BytesRef> contexts = null;
    /**
     * The weight of the term.
     */
    public long weight = 1;
    /**
     * An optional payload.
     */
    public BytesRef payload = null;
  }
  /**
   * The wrapped iterator.
   */
  private final InputIterator _wrapped;
  /**
   * The collected items.
   */
  private final TreeMap<BytesRef, Data> _items = new TreeMap<>();
  /**
   * Item iterator iterator.
   */
  private Iterator<Map.Entry<BytesRef, Data>> _itemIter = null;
  /**
   * Current item.
   */
  private Map.Entry<BytesRef, Data> _item = null;

  /**
   * Constructs a new UniqueInputIterator that wraps the given iterator.
   *
   * @param iter_ an input iterator.
   */
  public UniqueInputIterator(InputIterator iter_) {
    _wrapped = iter_;
  }

  /**
   * Collects the unique items from the wrapped iterator.
   */
  private void collectItems() throws IOException {
    BytesRef curr = _wrapped.next();
    while (curr != null) {
      Data data = _items.get(curr);
      if (data == null) {
        data = new Data();
        data.contexts = _wrapped.contexts();
        data.payload = _wrapped.payload();
        data.weight = _wrapped.weight();
        _items.put(curr, data);
      } else {
        updateData(curr, data, _wrapped);
      }

      curr = _wrapped.next();
    }
  }

  /**
   * Handler method for same items. Called for each occurrence of an item. You
   * can use this method to update the stored data eg. set the weight.
   *
   * The default implementation does nothing.
   *
   * @param item_ the current item in the interator.
   * @param itemData_ the currently stored data from.
   * @param iter_ the wrapped itarator.
   */
  protected void updateData(BytesRef item_, Data itemData_,
    InputIterator iter_) {
  };

  @Override
  public BytesRef next() throws IOException {
    if (_itemIter == null) {
      collectItems();

      _itemIter = _items.entrySet().iterator();
    }
    
    if (_itemIter.hasNext()) {
      _item = _itemIter.next();
      return _item.getKey();
    }

    _item = null;
    return null;
  }

  @Override
  public Comparator<BytesRef> getComparator() {
    return _wrapped.getComparator();
  }

  @Override
  public Set<BytesRef> contexts() {
    return _item.getValue().contexts;
  }

  @Override
  public boolean hasContexts() {
    return contexts() != null;
  }

  @Override
  public boolean hasPayloads() {
    return payload() != null;
  }

  @Override
  public BytesRef payload() {
    return _item.getValue().payload;
  }

  @Override
  public long weight() {
    return _item.getValue().weight;
  }
}

