package model.enums;

public enum InitializerKind {
  INSTANCE("instance"),
  STATIC("static");

  private final String name;

  InitializerKind(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }
}
