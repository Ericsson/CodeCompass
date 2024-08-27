package cc.search.common.ipc;

import cc.search.common.config.CommonOptions;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.thrift.TException;
import org.apache.thrift.TProcessor;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolFactory;
import org.apache.thrift.transport.TIOStreamTransport;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

/**
 * Helper class for thrift based IPC communication.
 */
public class IPCProcessor implements AutoCloseable {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * Processor for thrift messages.
   */
  private final TProcessor _processor;
  /**
   * Transport object for incoming messages.
   */
  private final TTransport _inTransport;
  /**
   * Transport object for outgoing messages.
   */
  private final TTransport _outTransport;
  /**
   * Protocol object for incoming messages.
   */
  private final TProtocol _inProtocol;
  /**
   * Protocol object for outgoing messages.
   */
  private final TProtocol _outProtocol;
  /**
   * Ture if serving is enabled.
   */
  private boolean _continueServing = true;
  
  /**
   * Creates a new processor.
   * 
   * @param options_ app options.
   * @param processor_  a thrift processor.
   * @throws java.io.FileNotFoundException 
   */
  public IPCProcessor(CommonOptions options_, TProcessor processor_)
    throws FileNotFoundException {
    _processor = processor_;
    
    TProtocolFactory factory = new TBinaryProtocol.Factory();
    TTransport inTransport = null;
    TTransport outTransport = null;
    TProtocol inProtocol = null;
    TProtocol outProtocol = null;

    try {
      inTransport = new TIOStreamTransport(new FileInputStream(getFileNameFromFd(options_.ipcInFd)));
      outTransport = new TIOStreamTransport(new FileOutputStream(getFileNameFromFd(options_.ipcOutFd)));
      inProtocol = factory.getProtocol(inTransport);
      outProtocol = factory.getProtocol(outTransport);
    } catch (TTransportException ex) {
      _log.log(Level.SEVERE, "An error occured during the initialization of IO stream transports!", ex);
    } finally {
      _inTransport = inTransport;
      _outTransport = outTransport;
      _inProtocol = inProtocol;
      _outProtocol = outProtocol;
    }
  }
  
  /**
   * Serve IPC (thrift) messages.
   */
  public void serve() {
    _continueServing = true;
    
    try {
      _inTransport.open();
      _outTransport.open();
    } catch (TTransportException ex) {
      _log.log(Level.SEVERE, "Opening transoprt failed!", ex);
      return;
    }
    
    try {
      while (_continueServing) {
        _processor.process(_inProtocol, _outProtocol);
      }
    } catch (TTransportException ex) {
      _log.log(Level.SEVERE, "Client died!", ex);
    } catch (TException ex) {
      _log.log(Level.SEVERE, "Something went wrong!", ex);
    } catch (Exception x) {
      _log.log(Level.SEVERE, "Error occurred during processing of message.", x);
    }
    close();
  }
  
  /**
   * Stop serving.
   */
  public void stopServe() {
    _continueServing = false;
  }

  /**
   * Java does not supports using file descriptors so we have to cheat. It is a
   * Linux only solution: give back a file that represents the given fid.
   * 
   * @param fd_ file descriptor.
   * @return path to a file that represents the descriptor.
   */
  private String getFileNameFromFd(int fd_) {
    return "/proc/self/fd/" + Integer.toString(fd_);
  }

  @Override
  public void close() {
    stopServe();
    if (_inTransport != null) {
      _inTransport.close();
    }
    if (_outTransport != null) {
      _outTransport.close();
    }
  }
}
