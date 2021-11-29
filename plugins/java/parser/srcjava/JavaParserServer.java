package parser.srcjava;

import org.apache.log4j.BasicConfigurator;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;

import java.util.logging.Level;

import static logger.Logger.LOGGER;
import cc.parser.java.JavaParserService;

public class JavaParserServer {
  public static JavaParser javaParser;
  public static JavaParserService.Processor<?> processor;

  public static void main(String [] args) {
    BasicConfigurator.configure();

    try {
      javaParser = new JavaParser();
      processor = new JavaParserService.Processor<>(javaParser);

      Runnable threadPool = () -> threadPool(processor);
      new Thread(threadPool).start();
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "[javaparser] Java server starting failed!");
    }
  }

  public static void threadPool(JavaParserService.Processor<?> processor) {
    try {
      TServerTransport serverTransport = new TServerSocket(9090);
      TThreadPoolServer.Args a =
        new TThreadPoolServer.Args(serverTransport).processor(processor);
      int threadNum = Integer.parseInt(System.getProperty("threadNum"));

      a.minWorkerThreads(1);
      a.maxWorkerThreads(threadNum);

      TThreadPoolServer server = new TThreadPoolServer(a);

      server.serve();
    } catch (Exception e) {
      e.printStackTrace();
      LOGGER.log(
        Level.SEVERE, "[javaparser] Java server starting failed!");
    }
  }
}
