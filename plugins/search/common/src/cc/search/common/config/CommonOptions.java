package cc.search.common.config;

import java.util.Iterator;
import java.util.List;

import org.apache.lucene.store.LockFactory;
import org.apache.lucene.store.NativeFSLockFactory;

import cc.search.common.NFSFriendlyLockFactory;

/**
 * Common options for search applications.
 */
public abstract class CommonOptions {
  /**
   * Index database path
   */
  public String indexDirPath;
  /**
   * Logging file path
   * (if there is no file logging, empty string)
   */
  public String logFilePath = "";
  /**
   * Input file descriptor for thrift IPC.
   */
  public int ipcInFd;
  /**
   * Output file descriptor for thrift IPC.
   */
  public int ipcOutFd;
  /**
   * Use SimpleFileLock in Lucene. It's more NFS friendly, but not as stable as
   * a native lock. This option is basically because artf448170.
   */
  public boolean useSimpleLock = false;
  /**
   * Cleanup locks on start. When using SimpleFileLocks we have to cleanup the
   * lock files after an "abnormal process termination" (i.e.: kill or crash).
   */
  public boolean cleanupLocks = false;

  /**
   * Checks all required fields.
   * @throws cc.search.common.config.InvalidValueException
   */
  protected void validate() throws InvalidValueException {
    if (indexDirPath == null || indexDirPath.isEmpty()) {
      throw new InvalidValueException("No index database path given!");
    }
    
    if (ipcInFd == 0) {
      throw new InvalidValueException("No ipcInFd!");
    }
    
    if (ipcOutFd == 0) {
      throw new InvalidValueException("No ipcOutFd!");
    }
  }

  /**
   * Sets values from command line arguments.
   *
   * @param args_ command line arguments.
   * @throws cc.search.common.config.InvalidValueException
   * @throws cc.search.common.config.UnknownArgumentException
   */
  protected void setFromCommandLineArguments(List<String> args_)
    throws InvalidValueException, UnknownArgumentException {

    Iterator<String> argIter = args_.iterator();
    while (argIter.hasNext()) {
      String arg = argIter.next();

      switch (arg) {
        case "-indexDB":
          if (!argIter.hasNext()) {
            throw new InvalidValueException("No path for -indexDB");
          } else {
            argIter.remove();
            indexDirPath = argIter.next();
            argIter.remove();
          }
          break;
        case "-logTarget":
          if (!argIter.hasNext()) {
            throw new InvalidValueException("No path for -logTarget");
          } else {
            argIter.remove();
            logFilePath = argIter.next();
            argIter.remove();
          }
          break;
        case "-ipcInFd":
          if (!argIter.hasNext()) {
            throw new InvalidValueException("-ipcInFd is empty");
          } else {
            argIter.remove();
            ipcInFd = Integer.parseInt(argIter.next());
            argIter.remove();
          }
          break;
        case "-ipcOutFd":
          if (!argIter.hasNext()) {
            throw new InvalidValueException("-ipcOutFd is empty");
          } else {
            argIter.remove();
            ipcOutFd = Integer.parseInt(argIter.next());
            argIter.remove();
          }
          break;
        case "-useSimpleFileLock":
          useSimpleLock = true;
          argIter.remove();
          break;
        case "-cleanupLocks":
          cleanupLocks = true;
          argIter.remove();
          break;
      }
    }
  }
  
  /**
   * Creates a new lock factory based on configuration. The default behavior is
   * to create a NativeFSLockFactory but for NFS it isn't working (the locking
   * times out and throws an exception) so if useSimpleLock is true then this
   * method creates an NFSFriendlyLockFactory.
   * 
   * This method is introduced because of artf448170.
   * 
   * @return a new lock factory.
   */
  public LockFactory createLockFactory() {
    if (useSimpleLock) {
      return new NFSFriendlyLockFactory(cleanupLocks);
    } else {
      return new NativeFSLockFactory();
    }
  }

  /**
   * Returns a command line help message for the user.
   * 
   * @return help message.
   */
  public static String getUsage() {
    return "Command line arguments:\n"
      + "\t-indexDB path\n\t\tPath of index database.\n"
      + "\t-ipcInFd fd\n\t\tFile descriptor for IPC IN.\n"
      + "\t-ipcOutFd id\n\t\tFile descriptor for IPC OUT.\n"
      + "\t-useSimpleFileLock\n\t\tUse NFS friendly file locks.\n"
      + "\t-logTarget\n\t\tPath to logging file.\n"
      + "\t-cleanupLocks\n\t\tCleanup locks before first lock..\n";
  }
}