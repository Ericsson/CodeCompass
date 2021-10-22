package logger;

import java.io.IOException;
import java.io.InputStream;
import java.util.logging.LogManager;

public abstract class Logger {
  public static final java.util.logging.Logger LOGGER = initLogger();

  private static java.util.logging.Logger initLogger() {
    InputStream stream = Logger.class.getClassLoader().
      getResourceAsStream("META-INF/logging.properties");

    try {
      LogManager.getLogManager().readConfiguration(stream);
      return java.util.logging.Logger.getLogger(Logger.class.getName());

    } catch (IOException e) {
      System.err.println(
        "Logger initialization for Java plugin has been failed."
      );
    }
    return null;
  }
}
