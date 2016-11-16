package cc.search.indexer.app;

import cc.search.common.config.CommonOptions;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Program options for Search indexer.
 */
final class Options extends CommonOptions {
  /**
   * Index directory write mode.
   */
  public enum OpenMode {
    CREATE,
    REPLACE_OLD,
    MERGE
  }

  /**
   * Index database open mode.
   */
  public OpenMode indexOpenMode = OpenMode.CREATE;
  
  @Override
  protected void setFromCommandLineArguments(List<String> args_)
    throws InvalidValueException, UnknownArgumentException {
    
    super.setFromCommandLineArguments(args_);
    
    Iterator<String> argIter = args_.iterator();
    while (argIter.hasNext()) {
      String arg = argIter.next();
      
      switch (arg) {
        case "-create":
          indexOpenMode = OpenMode.CREATE;
          break;
        case "-replaceExisting":
          indexOpenMode = OpenMode.REPLACE_OLD;
          break;
        case "-merge":
          indexOpenMode = OpenMode.MERGE;
          break;
        default:
          throw new UnknownArgumentException(arg);
      }
    }
  }
  
  /**
   * Returns a command line help message for the user.
   * 
   * @return help message.
   */
  public static String getUsage() {
    return CommonOptions.getUsage()
      + "\t-create\n\t\tOverwrite the index database if already exists or create one.\n"
      + "\t-append\n\t\tDo not overwrite the index database if already exists, just append to the documents.\n"
      + "\t-replaceExisting\n\t\tOverwrite the index database. (Not implemented)\n"
      + "\n\n"
      + "The indexer reads the file paths line-by-line from the given source \n"
      + "(-indexFilesList) which is the standard input by default. ";
  }

  /**
   * Builds an instance from command line arguments.
   *
   * @param args_ command line arguments.
   */
  public Options(String[] args_)
    throws UnknownArgumentException, InvalidValueException {
    
    ArrayList<String> args = new ArrayList<>(Arrays.asList(args_));
    setFromCommandLineArguments(args);

    validate();
  }
}
