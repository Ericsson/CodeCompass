package cc.search.analysis.tags;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

/**
 * Container for tags in a file.
 */
public final class Tags implements Serializable {
  /**
   * Serial version id.
   */
  private static final long serialVersionUID = 5550393602933769039L;
  
  /**
   * Map from start offsets to tags.
   * 
   * From Carmack release we allow multiple tags on the same offset.
   */
  private final HashMap<Integer, List<Tag>> _startOffsetToTag;
  
  /**
   * Represents a tag with its offset.
   */
  public static class TagItem {
    /**
     * The offset of the tag.
     */
    public final Integer offset;
    /**
     * The tag itself.
     */
    public final Tag tag;
    /**
     * Creates a new TagItem
     * 
     * @param offset_ the offset of the tag.
     * @param tag_ the tag itself.
     */
    public TagItem(Integer offset_, Tag tag_) {
      offset = offset_;
      tag = tag_;
    }
  }
  
  /**
   * Creates an empty object.
   */
  public Tags() {
    _startOffsetToTag = new HashMap<>();
  }
  
  /**
   * Adds a new tag. You can add multiple tags to same offset.
   * 
   * @param tag_ tag.
   * @param startOffset_ Start offset.
   */
  public void add(Tag tag_, int startOffset_) {
    List<Tag> tagList = _startOffsetToTag.get(startOffset_);
    if (tagList == null) {
      tagList = new ArrayList<Tag>();
      _startOffsetToTag.put(startOffset_, tagList);
    }
    
    tagList.add(tag_);
  }
  
  /**
   * Returns a tag on the given start offset or null if there is no tag on the
   * specified offset.
   * 
   * @param offset_ start offset.
   * @return tag or null if not found.
   */
  public Tag getByOffset(int offset_) {
    List<Tag> tagList = _startOffsetToTag.get(offset_);
    if (tagList == null || tagList.isEmpty()) {
      return null;
    }
    
    return tagList.get(0);
  }
  
  /**
   * Returns all tags on the given start offset or null if there is no tag on
   * the specified offset.
   * 
   * @param offset_ start offset.
   * @return tag or null if not found.
   */
  public List<Tag> getAllByOffset(int offset_) {
    List<Tag> tagList = _startOffsetToTag.get(offset_);
    if (tagList == null || tagList.isEmpty()) {
      return null;
    }
    
    return tagList;
  }

  /**
   * Creates a new container with all tags in it (ordered by offsets ASC).
   * 
   * @return the tags in a new container.
   */
  public Collection<TagItem> getAllTags() {
    final ArrayList<TagItem> resultList = new ArrayList<>(
      _startOffsetToTag.size() * 2);
    
    final Iterator<Integer> keyIter = _startOffsetToTag.keySet().iterator();
    while (keyIter.hasNext()) {
      final Integer key = keyIter.next();
      final List<Tag> tags = _startOffsetToTag.get(key);
      
      for (final Tag tag : tags) {
        resultList.add(new TagItem(key, tag));
      }
    }
    
    return resultList;
  }
  
  /**
   * @return a collection of original kind texts.
   */
  public Collection<String> calculateOriginalKindSet() {
    final HashSet<String> kinds = new HashSet<>(40);
    
    final Collection<TagItem> tags = getAllTags();
    for (final TagItem tagItem : tags) {
      kinds.add(tagItem.tag.kind);
    }
    
    return kinds;
  }
  
  /**
   * Serializes the object.
   * 
   * @return serialized data.
   * @throws IOException 
   */
  public byte[] serialize() throws IOException {
    ByteArrayOutputStream bs = new ByteArrayOutputStream();
    ObjectOutputStream os = new ObjectOutputStream(bs);
    os.writeObject(this);
    return bs.toByteArray();
  }
  
  /**
   * Deserializes a Tags object.
   * 
   * @param bytes_ data.
   * @return a deserialized Tags object. 
   * @throws IOException
   * @throws ClassNotFoundException 
   */
  public static Tags deserialize(byte[] bytes_)
    throws IOException, ClassNotFoundException {
    ByteArrayInputStream bs = new ByteArrayInputStream(bytes_);
    ObjectInputStream os = new ObjectInputStream(bs);
    Object obj = os.readObject();
    
    return (Tags) obj;
  }
}
