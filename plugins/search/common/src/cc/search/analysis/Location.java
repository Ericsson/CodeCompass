package cc.search.analysis;

import java.io.Serializable;

/**
 * Describes the location of a Token, Tag or other entity.
 *
 * Locations in the search indexer and service are includive on both end!
 */
public final class Location implements Serializable {
  /**
   * Line number (starting from 1)
   */
  public final int line;
  /**
   * Start column (starting from 1)
   */
  public final int startColumn;
  /**
   * End column (starting from 1)
   */
  public final int endColumn;
  
  /**
   * @param line_ line number.
   * @param startColumn_ start column.
   * @param endColumn_ end column.
   */
  public Location(int line_, int startColumn_, int endColumn_) {
    line = line_;
    startColumn = startColumn_;
    endColumn = endColumn_;
    
    // Some basic validation
    if (line < 1 || startColumn < 1 || startColumn > endColumn) {
      throw new IllegalArgumentException();
    } 
  }
  
  /**
   * @param other_ an other location.
   * @return true if this location is inside the given location.
   */
  public boolean isInside(Location other_) {
    return other_.line == line && other_.startColumn <= startColumn &&
      other_.endColumn >= endColumn;
  }
  
  @Override
  public int hashCode() {
    int hash = 7;
    hash = 37 * hash + this.line;
    hash = 37 * hash + this.startColumn;
    hash = 37 * hash + this.endColumn;
    return hash;
  }

  @Override
  public boolean equals(Object obj_) {
    if (obj_ == null) {
      return false;
    }
    if (getClass() != obj_.getClass()) {
      return false;
    }
    final Location other = (Location) obj_;
    if (this.line != other.line) {
      return false;
    }
    if (this.startColumn != other.startColumn) {
      return false;
    }
    return this.endColumn == other.endColumn;
  }
  
  @Override
  public String toString() {
    return "Location{" + "line=" + line + ", startColumn=" + startColumn +
      ", endColumn=" + endColumn + '}';
  }
}
