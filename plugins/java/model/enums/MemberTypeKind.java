package model.enums;

public enum MemberTypeKind {
  TYPE("Type"),
  CONSTRUCTOR("Constructor"),
  FIELD("Field"),
  METHOD("Method"),
  ENUM("Enum"),
  ENUM_CONSTANT("Enum constant");

  private final String name;

  MemberTypeKind(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
