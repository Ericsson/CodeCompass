package cc.search.common;

import java.io.IOException;
import java.util.HashSet;
import org.apache.lucene.store.Lock;
import org.apache.lucene.store.SimpleFSLockFactory;

/**
 * An NFS friendly lock factory with cleanup feature. After an abnormal exit a
 * file lock could (and will) remain on the disc. As a workaround, this lock
 * factroy deletes the previous lock (if any) on the first makeLock(String)
 * request (if it was enabled in the constructor).
 * 
 * @author Alex Ispanovics
 */
public class NFSFriendlyLockFactory extends SimpleFSLockFactory{
  /**
   * Enables / disables the lock cleanup.
   */
  private final boolean _cleanOnFirstUse;
  /**
   * Contains the locks (with prefix) that was locked before.
   */
  private final HashSet<String> _lockHistory = new HashSet<String>();
  
  /**
   * Constructs a NFSFriendlyLockFactory.
   * 
   * @param cleanOnFirstUse_ enables / disables the lock cleanup
   */
  public NFSFriendlyLockFactory(boolean cleanOnFirstUse_) {
    _cleanOnFirstUse = cleanOnFirstUse_;
  }
  
  @Override
  public Lock makeLock(String lockName_) {
    if (_cleanOnFirstUse) {
      // Add lock prefix to name
      String fullLockName;
      if (lockPrefix != null) {
        fullLockName = lockPrefix + "-" + lockName_;
      } else {
        fullLockName = lockName_;
      }
      
      if (!_lockHistory.contains(fullLockName)) {
        try {
          clearLock(lockName_);
        } catch (IOException ex) {
          // try to continue
        }
        
        _lockHistory.add(fullLockName);
      }
    }
    
    return super.makeLock(lockName_);
  }
}
