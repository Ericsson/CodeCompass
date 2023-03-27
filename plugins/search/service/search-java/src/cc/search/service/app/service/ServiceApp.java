package cc.search.service.app.service;

import cc.search.common.FileLoggerInitializer;
import cc.search.common.ipc.IPCProcessor;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;
import cc.service.search.SearchService;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The search service application.
 */
public class ServiceApp extends SearchHandler {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * IPC message processor.
   */
  private final IPCProcessor _processor;

  /**
   * @param options_ command line options.
   * @throws IOException 
   */
  private ServiceApp(ServiceAppOptions options_) throws IOException {
    super(options_);
    
    _processor = new IPCProcessor(options_, new SearchService.Processor(this));
  }

  @Override
  public void pleaseStop() {
    _processor.stopServe();
  }

  @Override
  public void close() {
    _processor.close();
    
    super.close();
  }
  
  public static void main(String[] args_)  {
    try {
    // Initialize file logger
    ServiceAppOptions options = new ServiceAppOptions(args_);
    FileLoggerInitializer.addFileOutput(options, _log, "search");

    _log.info("Search server started!");
      try (ServiceApp app =
             new ServiceApp(options)) {

        _log.log(Level.INFO, "Start serving search for index {0}",
          app._options.indexDirPath);

        app._processor.serve();
      } catch (IOException e) {
        _log.log(Level.SEVERE, "Fatal error!", e);
        System.exit(-1);
      } 
    } catch (UnknownArgumentException | InvalidValueException e) {
      _log.log(Level.SEVERE, "Fatal error!", e);
      System.exit(-1);
    } catch (Exception ex) {
      _log.log(Level.SEVERE, "Something bad happened :-(", ex);
    } finally {
      _log.info("Search server finished!");
    }
  }
}
