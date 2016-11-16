package cc.search.common.config;

public class UnknownArgumentException extends Exception {
  public UnknownArgumentException(String arg_) {
    super("Unknown argument: " + arg_);
  }
}
