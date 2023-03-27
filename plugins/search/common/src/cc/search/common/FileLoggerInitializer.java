package cc.search.common;

import cc.search.common.config.CommonOptions;

import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.FileHandler;
import java.util.logging.SimpleFormatter;

/**
 * Adds file to logging when needed
 */
public class FileLoggerInitializer {
  public static void addFileOutput(CommonOptions options_, Logger log_, String pluginName_) {
    String filePathField = options_.logFilePath + pluginName_ + ".log";
    if (!options_.logFilePath.isEmpty()) {
      try {
        FileHandler fileHandler = new FileHandler(filePathField, false);
        SimpleFormatter formatter = new SimpleFormatter();
        fileHandler.setFormatter(formatter);
        log_.addHandler(fileHandler);
        log_.info("Logging started to file: " + filePathField);
      } catch (Exception ex) {
        log_.log(Level.WARNING, "Could not add logs to file: " + filePathField, ex);
      }
    }
  }
}
