package parser.srcjava;

import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;

import java.util.logging.Level;

import static logger.Logger.LOGGER;
import cc.parser.java.JavaParserService;

public class JavaParserServer {
  public static JavaParser javaParser;
  public static JavaParserService.Processor<?> processor;

  public static void main(String [] args) {
    try {
      javaParser = new JavaParser();
      processor = new JavaParserService.Processor<>(javaParser);

      Runnable simple = () -> simple(processor);
      new Thread(simple).start();

      LOGGER.log(Level.INFO, "[javaparser] Java server started!");
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "[javaparser] Java server starting failed!");
    }
  }

  public static void simple(JavaParserService.Processor<?> processor) {
    try {
      TServerTransport serverTransport = new TServerSocket(9090);
      TServer server =
        new TSimpleServer(new Args(serverTransport).processor(processor));

      server.serve();
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "[javaparser] Java server starting failed!");
    }
  }
}
