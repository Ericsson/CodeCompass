package model.enums;

public enum Visibility {
  PRIVATE("private", 0),
  PACKAGE_PRIVATE("package-private", 1),
  PROTECTED("protected", 2),
  PUBLIC("public", 3);

  private final String name;
  private final int value;

  Visibility(String name, int value) {
    this.name = name;
    this.value = value;
  }

  public String getName() {
    return name;
  }

  public int getValue() {
    return value;
  }
}
