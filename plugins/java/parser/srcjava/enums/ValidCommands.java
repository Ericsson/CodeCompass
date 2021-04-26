package parser.srcjava.enums;

public enum ValidCommands {
  JAVAC("javac");
  /*
  FUTURE PLANS:
  MVN("mvn");
  etc...
  */

  private final String name;

  ValidCommands(String name) {
    this.name = name;
  }

  // Getter
  public String getName() {
    return name;
  }
}
