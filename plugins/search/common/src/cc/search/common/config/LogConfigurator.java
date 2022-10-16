package cc.search.common.config;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

/**
 * This class configures the java.util.logging.LogManager. 
 */
public class LogConfigurator {
  /**
   * Configures the logging options based on JVM arguments.
   */
  public LogConfigurator() {
    final LogManager logManager = LogManager.getLogManager();
    
    try {
      String config = createConfig();

      ByteArrayInputStream cfgStream = new ByteArrayInputStream(
        config.getBytes());

      logManager.readConfiguration(cfgStream);
      
    } catch (IOException | SecurityException ex) {
      Logger.getLogger("GLOBAL_LOGGER").log(Level.SEVERE,
        "Failed to configure custom log propeties!", ex);
    }
  }
  
  /**
   * Creates a configuration in LogManager config format.
   * 
   * @return log configuration.
   */
  private static String createConfig() {
    String logLevel = System.getProperty("cc.search.logLevel");
    if (logLevel == null) {
      logLevel = Level.INFO.getName();
    }
    
    return
      "handlers = java.util.logging.ConsoleHandler\n" + 
      "cc.search.level = " + logLevel + "\n" +
      "java.util.logging.ConsoleHandler.level = ALL\n";
  }
}
