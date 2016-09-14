package cc.search.analysis.tags;

import cc.search.analysis.Location;
import java.io.Serializable;
import java.util.Objects;

/**
 * Describes tag (eg symbol definition).
 */
public final class Tag implements Serializable {
  /**
   * Serial version id.
   */
  private static final long serialVersionUID = -2884956299106710632L;

  /**
   * (Generic) Kind of a tag
   */
  public static enum Kind {
    /**
     * Some exotic tag.
     */
    Other,
    /**
     * Type definition: class, struct, interface, etc.
     */
    Type,
    /**
     * Macro.
     */
    Macro,
    /**
     * constants, enumerator (values inside an enumeration).
     */
    Constant,
    /**
     * Function, method, subroutine (usually only its name).
     */
    Function,
    /**
     * Field in a structure/class.
     */
    Field,
    /**
     * (Function) prototype (I`m not sure what is this). It is usually means an
     * external function.
     */
    Prototype,
    /**
     * (Global or local) variable.
     */
    Variable,
    /**
     * Label.
     */
    Label,
    /**
     * C++ Namespace, Java package.
     */
    Module
  }
  /**
   * Generic kind of this tag.
   */
  public final Kind genericKind;
  /**
   * Location of this tag.
   */
  public final Location location;
  /**
   * The actual tag as its in the source content.
   */
  public final String text;
  /**
   * The original kind of this tag.
   */
  public final String kind;
  
  /**
   * @param kind_ kind of this tag.
   * @param location_ location of this tag.
   * @param text_ tag as text.
   * @param genKind_ generic kind.
   */
  public Tag(Location location_, String text_, String kind_, Kind genKind_) {
    genericKind = genKind_;
    location = location_;
    text = text_;
    kind = kind_;
  }

  @Override
  public int hashCode() {
    int hash = 7;
    hash = 83 * hash + Objects.hashCode(this.genericKind);
    hash = 83 * hash + Objects.hashCode(this.location);
    hash = 83 * hash + Objects.hashCode(this.text);
    hash = 83 * hash + Objects.hashCode(this.kind);
    return hash;
  }

  @Override
  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    if (getClass() != obj.getClass()) {
      return false;
    }
    final Tag other = (Tag) obj;
    if (this.genericKind != other.genericKind) {
      return false;
    }
    if (!Objects.equals(this.location, other.location)) {
      return false;
    }
    if (!Objects.equals(this.text, other.text)) {
      return false;
    }
    return Objects.equals(this.kind, other.kind);
  }
  
  @Override
  public String toString() {
    return "Tag{" + "genericKind=" + genericKind + ", location=" + location +
      ", text=" + text + ", tagKind=" + kind + '}';
  }
  
  
}
