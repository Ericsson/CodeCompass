package cc.search.analysis.tags;

import java.io.IOException;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A singleton manager class for managing tag generators in a multi-threaded
 * environment.
 */
public final class TagGeneratorManager implements AutoCloseable {
  /**
   * Logger.
   */
  private final static Logger _log = Logger.getLogger("GLOBAL_LOGGER");
  /**
   * Singleton instance.
   */
  private static TagGeneratorManager _instance = null;
  /**
   * Tag generator cache.
   */
  private final Queue<TagGenerator> _cache;
  
  /**
   * Initializes the members.
   */
  private TagGeneratorManager() {
    _cache = new ConcurrentLinkedQueue<>();
  }
  
  /**
   * Returns a new/cached tag generator. This method is thread safe.
   * 
   * @return a tag generator.
   * @throws IOException 
   */
  public TagGenerator getGenerator() throws IOException {
    TagGenerator gen = _cache.poll();
    if (gen == null) {
      gen = new SourceTagGenerator();
    }
    
    return gen;
  }
  
  /**
   * Releases a tag generator previously got by get() method.
   * 
   * @param gen_ a tag generator.
   */
  public void releaseGenerator(TagGenerator gen_) {
    _cache.add(gen_);
  }
  
  /**
   * Init singleton. Must be called from the main thread.
   */
  public static void init() {
    if (_instance != null) {
      _log.log(Level.SEVERE, "Double init on TagGeneratorManager!");
    } else {
      _instance = new TagGeneratorManager();
    }
  }
  
  /**
   * Destroys the singleton. Must be called from the main thread.
   */
  public static void destroy() {
    if (_instance == null) {
      _log.log(Level.SEVERE, "Double destroy on TagGeneratorManager!");
    } else {
      _instance.close();
      _instance = null;
    }
  }
  
  /**
   * Returns the singleton instance.
   * 
   * @return the global manager.
   */
  public static TagGeneratorManager get() {
    return _instance;
  }

  @Override
  public void close() {
    for (TagGenerator gen : _cache) {
      try {
        gen.close();
      } catch (Exception ex) {
        _log.log(Level.WARNING, "Failed to close a tag generator!", ex);
      }
    }
  }

}
