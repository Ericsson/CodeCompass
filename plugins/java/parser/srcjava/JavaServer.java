package parser.srcjava;

import org.apache.log4j.BasicConfigurator;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;

import java.util.logging.Level;

import static parser.srcjava.Logger.LOGGER;
import cc.parser.java.JavaParserService;

public class JavaServer {
  public static JavaParser javaParser;
  public static JavaParserService.Processor<?> processor;

  public static void main(String [] args) {
    try {
      javaParser = new JavaParser(args[0]);
      processor = new JavaParserService.Processor<>(javaParser);

      Runnable simple = new Runnable() {
        public void run() {
          simple(processor);
        }
      };

      new Thread(simple).start();
    } catch (Exception x) {
      LOGGER.log(
        Level.SEVERE, "Starting Thrift server for Java plugin failed");
    }
  }

  public static void simple(JavaParserService.Processor<?> processor) {
    try {
      TServerTransport serverTransport = new TServerSocket(9090);
      TServer server =
        new TSimpleServer(new Args(serverTransport).processor(processor));

      LOGGER.log(Level.INFO, "Starting Thrift server for Java plugin");
      server.serve();
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "Starting Thrift server for Java plugin failed");
    }
  }
}
