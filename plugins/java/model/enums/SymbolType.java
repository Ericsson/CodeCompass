package model.enums;

public enum SymbolType {
  VARIABLE("Variable", "variable"),
  CONSTRUCTOR("Constructor", "constructor"),
  METHOD("Method", "method"),
  ENUM("Enum", "enum"),
  ENUM_CONSTANT("Enum constant", "enum-constant"),
  TYPE("Type", "type"),
  INITIALIZER("Initializer", "initializer"),
  FILE("File", "file");

  private final String name;
  private final String value;

  SymbolType(String name, String value) {
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
