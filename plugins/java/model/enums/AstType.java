package model.enums;

public enum AstType {
  STATEMENT("Statement", "statement"),
  DECLARATION("Declaration", "declaration"),
  DEFINITION("Definition", "definition"),
  USAGE("Usage", "usage"),
  READ("Read", "read"),
  WRITE("Write", "write");

  private final String name;
  private final String value;

  AstType(String name, String value) {
    this.name = name;
    this.value = value;
  }

  public String getName() {
    return name;
  }

  public String getValue() {
    return value;
  }
}
