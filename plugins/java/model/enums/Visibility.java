package model.enums;

public enum Visibility {
  PRIVATE("private"),
  PACKAGE_PRIVATE("package-private"),
  PROTECTED("protected"),
  PUBLIC("public");

  private final String name;

  Visibility(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
