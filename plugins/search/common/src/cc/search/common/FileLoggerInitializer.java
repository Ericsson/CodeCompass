package cc.search.common;

import cc.search.common.config.CommonOptions;

import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.FileHandler;
import java.util.logging.SimpleFormatter;

/**
 * Adds file to logging when needed
 */
public class FileLoggerInitializer implements Runnable {
  /**
   * The name of the "path" field. Path contains the full path of the file.
   */
  private static String _filePathField = "";
  /**
   * The logger that needs the file handler
   */
  private static Logger _log;

  public FileLoggerInitializer(CommonOptions options_, Logger log_) {
    _filePathField = options_.logFilePath;
    _log = log_;
  }

  public void run() {
    if (!_filePathField.isEmpty()) {
      try {
        FileHandler fileHandler = new FileHandler(_filePathField, true); // append mode
        SimpleFormatter formatter = new SimpleFormatter();
        fileHandler.setFormatter(formatter);
        _log.addHandler(fileHandler);
        _log.info("Logging started to file: " + _filePathField);
      } catch (Exception ex) {
        _log.log(Level.WARNING, "Could not add logs to file: " + _filePathField, ex);
      }
    }
  }
}
