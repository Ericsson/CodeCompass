package model.enums;

import org.eclipse.jdt.core.dom.Modifier;

public enum Visibility {
  PRIVATE("private", Modifier.PRIVATE),
  PACKAGE_PRIVATE("package-private"),
  PROTECTED("protected", Modifier.PROTECTED),
  PUBLIC("public", Modifier.PUBLIC);

  private final String name;
  private int modifier;

  Visibility(String name) {
    this.name = name;
  }

  Visibility(String name, int modifier) {
    this.name = name;
    this.modifier = modifier;
  }

  public String getName() {
    return name;
  }

  public int getModifier() {
    return modifier;
  }
}
