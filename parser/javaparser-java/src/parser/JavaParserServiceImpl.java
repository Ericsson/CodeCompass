package parser;

import java.util.logging.Logger;
import java.util.logging.Level;
import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.net.Socket;

import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TTransportException;

import cc.parser.JavaParserArg;
import cc.parser.JavaParserService;
import cc.parser.JavaParsingResult;

import parser.Parser;

public class JavaParserServiceImpl implements JavaParserService.Iface {

  private TServer server = null;
  static private int initPort = 0;
  public Boolean isStarted = false;

  public static void main(String[] args)
  {
    if(args.length < 1)
    {
      System.out.println(
        "JavaParser: Port number must be given for initializer communication.");
    }
    initPort = Integer.parseInt(args[0]);

    JavaParserServiceImpl jpsi = new JavaParserServiceImpl();
    jpsi.run();
  }

  void start()
  {
    try
    {
      // starting Thrift server
      TServerSocket serverTransport = new TServerSocket(0);

      JavaParserService.Processor processor = new JavaParserService.Processor(
        new JavaParserServiceImpl());

      server = new TSimpleServer(new TSimpleServer.Args(serverTransport).
      processor(processor));

      int port = serverTransport.getServerSocket().getLocalPort();
      System.out.println("JavaParser: Starting server on port " + port + "...");

      // send back the obtained port number
      System.out.println("JavaParser: Socker port: " + initPort);
      Socket initSocket = new Socket("127.0.0.1", initPort);
      DataOutputStream dos = new DataOutputStream(initSocket.getOutputStream());
      
      dos.writeInt(port);
      dos.flush();
     
      dos.close();
      initSocket.close();

      isStarted = true;
    }
    catch(Exception ex)
    {
      System.out.println("JavaParser: " + ex.getMessage());
      ex.printStackTrace();
    }
  }

  void run()
  {
    if(!isStarted)
      start();

    server.serve();
  }

  public JavaParsingResult parse(JavaParserArg arg) 
  {
    return Parser.parse(arg);
  }

  public void stop()
  {
    // server.stop();
    isStarted = false;

    System.exit(0);
  }
}
