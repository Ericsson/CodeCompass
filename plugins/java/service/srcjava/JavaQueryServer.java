package service.srcjava;

import org.apache.log4j.BasicConfigurator;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;

import java.util.logging.Level;

import static logger.Logger.LOGGER;
import cc.service.java.JavaService;

public class JavaQueryServer {
  public static JavaQueryHandler javaQueryHandler;
  public static JavaService.Processor<?> processor;

  public static void main(String [] args) {
    BasicConfigurator.configure();

    try {
      javaQueryHandler = new JavaQueryHandler();
      processor = new JavaService.Processor<>(javaQueryHandler);

      Runnable simple = () -> simple(processor);
      new Thread(simple).start();
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "Java server starting failed!");
    }
  }

  public static void simple(JavaService.Processor<?> processor) {
    try {
      TServerTransport serverTransport = new TServerSocket(9090);
      TServer server =
        new TSimpleServer(new Args(serverTransport).processor(processor));

      server.serve();
    } catch (Exception e) {
      LOGGER.log(
        Level.SEVERE, "Java server starting failed!");
    }
  }
}
