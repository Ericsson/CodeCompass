package model.enums;

public enum MemberTypeKind {
  FIELD("Field"),
  METHOD("Method");

  private final String name;

  MemberTypeKind(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
