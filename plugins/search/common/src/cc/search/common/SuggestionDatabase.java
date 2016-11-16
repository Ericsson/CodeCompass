package cc.search.common;

import cc.search.common.config.CommonOptions;
import java.io.IOException;
import java.io.File;

/**
 * Describes the possible suggestion databases.
 */
public enum SuggestionDatabase {
  /**
   * Filename suggestion database.
   */
  FileName("filename", true),
  /**
   * Symbol (definition) suggestion database.
   */
  Symbol("symbols", false);

  /**
   * Name of the directory under the search dir for the suggestion database.
   */
  private final String _dirName;
  /**
   * Is file or directory database.
   */
  private final boolean _isFileDb;

  /**
   * Construct an enum value with the given database directory.
   *
   * @param dirName_ directory name.
   * @param isFileDb_ is a file db or a directory.
   */
  SuggestionDatabase(String dirName_, boolean isFileDb_) {
    _dirName = dirName_;
    _isFileDb = isFileDb_;
  }

  /**
   * Returns the path of the suggestion database using the given
   * configuration.
   *
   * @param opts_ common options.
   * @return the path of the suggestion database directory or file.
   */
  public File getDatabase(CommonOptions opts_) throws IOException {
    return getDatabase(opts_, false);
  }

  /**
   * Returns the path of the suggestion database using the given
   * configuration.
   *
   * TODO: the directory database cleaning is not implemented.
   *
   * @param opts_ common options.
   * @param clean_ if true, it will return an empty database.
   * @return the path of the suggestion database directory or file.
   */
  public File getDatabase(CommonOptions opts_, boolean clean_)
    throws IOException {
    final File dir = createDirectoryWithParents(opts_.indexDirPath +
      "/suggest/" + _dirName);

    if (_isFileDb) {
      return getDbFile(dir, clean_);
    }

    return dir;
  }

  /**
   * Creates the given directory with its parents. On any error the method
   * throws an IOException.
   *
   * @param dirPath_ a directory path.
   * @throws IOException on any error.
   * @return the File representation of ditPath_ parameter.
   */
  private File createDirectoryWithParents(String dirPath_) throws IOException {
    final File dir = new File(dirPath_);

    if (dir.exists()) {
      if (!dir.isDirectory()) {
        throw new IOException(dir.getAbsolutePath() + " must be a directory!");
      } else if (!dir.canWrite()) {
        throw new IOException(dir.getAbsolutePath() + " must be writable!");
      }
    } else if (!dir.mkdirs()) {
      throw new IOException("Failed to create " + dir.getAbsolutePath());
    }

    return dir;
  }

  /**
   * Returns the database file under the given directory. If the second
   * parameter is true then the file is deleted if exits.
   *
   * @param dir_ parent directory of the file.
   * @param delIfExists_ if this is true then the file is deleted before return.
   * @return the file.
   */
  private File getDbFile(File dir_, boolean delIfExists_) throws IOException {
    final File db = new File(dir_, "suggest.db");
    
    if (delIfExists_ && db.exists()) {
      if (!db.delete()) {
        throw new IOException("Could not delete " + db.getAbsolutePath());
      }
    }

    return db;
  }
}

