package cc.search.service.app.service;

import cc.search.common.config.CommonOptions;
import cc.search.common.config.InvalidValueException;
import cc.search.common.config.UnknownArgumentException;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * Command line options for the service application.
 */
final class ServiceAppOptions extends CommonOptions {

  /**
   * Builds an instance from command line arguments.
   *
   * @param args_ command line arguments.
   * @return an Options instance.
   */
  public ServiceAppOptions(String[] args_)
    throws InvalidValueException, UnknownArgumentException {

    ArrayList<String> args = new ArrayList<>(Arrays.asList(args_));
    setFromCommandLineArguments(args);

    validate();
  }
}
