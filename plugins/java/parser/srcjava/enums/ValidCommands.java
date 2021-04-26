package parser.srcjava.enums;

public enum ValidCommands {
  JAVAC("javac");
  /*
  FUTURE PLANS:
  MVN("mvn");
  etc...
  */

  private final String name;

  ValidCommands(String value) {
    this.name = value;
  }

  // Getter
  public String getName() {
    return name;
  }
}
