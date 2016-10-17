package cc.search.service.app.query;

import cc.search.common.config.CommonOptions;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Command line options for the Query application.
 */
final class QueryAppOptions extends CommonOptions {
  /**
   * Query string.
   */
  public String queryString;

  @Override
  protected void validate() throws InvalidValueException {
    super.validate();

    if (queryString == null || queryString.isEmpty()) {
      throw new InvalidValueException("No query string!");
    }
  }

  @Override
  protected void setFromCommandLineArguments(List<String> args_)
    throws InvalidValueException, UnknownArgumentException {

    super.setFromCommandLineArguments(args_);
    
    Iterator<String> argIter = args_.iterator();
    while (argIter.hasNext()) {
      String arg = argIter.next();

      switch (arg) {
        case "-query":
          if (!argIter.hasNext()) {
            throw new InvalidValueException("-query is empty");
          } else {
            argIter.remove();
            queryString = argIter.next();
            argIter.remove();
          }
          break;
        default:
          throw new UnknownArgumentException(arg);
      }
    }
  }

  /**
   * Builds an instance from command line arguments.
   *
   * @param args_ command line arguments.
   * @return an Options instance.
   */
  public QueryAppOptions(String[] args_)
    throws InvalidValueException, UnknownArgumentException {

    ArrayList<String> args = new ArrayList<>(Arrays.asList(args_));
    setFromCommandLineArguments(args);

    validate();
  }
}
