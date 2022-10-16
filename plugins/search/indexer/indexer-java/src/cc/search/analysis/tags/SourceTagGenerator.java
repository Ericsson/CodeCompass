package cc.search.analysis.tags;

import cc.search.indexer.Context;

import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Tag generator for source files, based on ctags.
 * 
 * Why we have two CTags instance here?
 *   The ctags program itself is a little buggy so if you have a C/C++ file and
 *   you enable the local variable (+l) kind, then some functions/methods will
 *   be missing from the ctags file. As a workaround this class wraps two ctags
 *   instance: one for generic tagging and one for the getting local variables.
 *   For more information see artf460600 in e-forge.
 */
public class SourceTagGenerator implements TagGenerator {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getLogger("GLOBAL_LOGGER");
  /**
   * Options array for initializing _artf460600CTags.
   */
  private static final ArrayList<String> _artf460600Options =
    new ArrayList<>(3);
  /**
   * A ctags process for generic tagging.
   */
  private CTags _genericCTags;
  /**
   * A ctags process for a workaround (artf460600, see in the class description)
   */
  private CTags _artf460600CTags;
  
  static {
    _artf460600Options.add("--C-kinds=l");
    _artf460600Options.add("--C++-kinds=l");
    _artf460600Options.add("--languages=C,C++");
  }
  
  /**
   * Start a ctags process.
   * 
   * @throws IOException 
   */
  public SourceTagGenerator() throws IOException {
    _genericCTags = new CTags();
    _artf460600CTags = new CTags(_artf460600Options);
  }
  
  /**
   * Starts or restarts ctags.
   * 
   * @throws IOException 
   */
  private void checkCTags() throws IOException {
    if (!_genericCTags.isRunning()) {
      // ctags exited somehow
      _log.log(Level.WARNING, "generic ctags exited abnormally!");
      _genericCTags.close();
      _genericCTags = new CTags();
    }
    
    if (!_artf460600CTags.isRunning()) {
      // ctags exited somehow
      _log.log(Level.WARNING, "artf460600 ctags exited abnormally!");
      _artf460600CTags.close();
      _artf460600CTags = new CTags(_artf460600Options);
    }
  }
  
  @Override
  public void close() {
    _genericCTags.close();
    _artf460600CTags.close();
  }
  
  @Override
  public void generate(Tags tags_, Context context_) throws IOException { 
    checkCTags();
    
    _genericCTags.generate(tags_, context_);
    _artf460600CTags.generate(tags_, context_);
  }

}
