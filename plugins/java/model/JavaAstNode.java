package model;

import model.enums.AstType;
import model.enums.SymbolType;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaAstNode\"")
public class JavaAstNode {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private long id;

  @Lob
  @Column(name = "\"astValue\"")
  private String astValue;

  @Column(name = "location_range_start_line")
  private long location_range_start_line;

  @Column(name = "location_range_start_column")
  private long location_range_start_column;

  @Column(name = "location_range_end_line")
  private long location_range_end_line;

  @Column(name = "location_range_end_column")
  private long location_range_end_column;

  @Column(name = "location_file")
  private long location_file;

  @Column(name = "\"entityHash\"")
  private long entityHash;

  @Column(name = "\"symbolType\"")
  @Enumerated(EnumType.ORDINAL)
  private SymbolType symbolType;

  @Column(name = "\"astType\"")
  @Enumerated(EnumType.ORDINAL)
  private AstType astType;

  @Column(name = "\"visibleInSourceCode\"")
  private boolean visibleInSourceCode;


  // Getters and setters

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public String getAstValue() {
    return astValue;
  }

  public void setAstValue(String astValue) {
    this.astValue = astValue;
  }

  public long getLocation_range_start_line() {
    return location_range_start_line;
  }

  public void setLocation_range_start_line(long location_range_start_line) {
    this.location_range_start_line = location_range_start_line;
  }

  public long getLocation_range_start_column() {
    return location_range_start_column;
  }

  public void setLocation_range_start_column(long location_range_start_column) {
    this.location_range_start_column = location_range_start_column;
  }

  public long getLocation_range_end_line() {
    return location_range_end_line;
  }

  public void setLocation_range_end_line(long location_range_end_line) {
    this.location_range_end_line = location_range_end_line;
  }

  public long getLocation_range_end_column() {
    return location_range_end_column;
  }

  public void setLocation_range_end_column(long location_range_end_column) {
    this.location_range_end_column = location_range_end_column;
  }

  public long getLocation_file() {
    return location_file;
  }

  public void setLocation_file(long location_file) {
    this.location_file = location_file;
  }

  public long getEntityHash() {
    return entityHash;
  }

  public void setEntityHash(long mangledNameHash) {
    this.entityHash = mangledNameHash;
  }

  public SymbolType getSymbolType() {
    return symbolType;
  }

  public void setSymbolType(SymbolType symbolType) {
    this.symbolType = symbolType;
  }

  public AstType getAstType() {
    return astType;
  }

  public void setAstType(AstType astType) {
    this.astType = astType;
  }

  public boolean isVisibleInSourceCode() {
    return visibleInSourceCode;
  }

  public void setVisibleInSourceCode(boolean visibleInSourceCode) {
    this.visibleInSourceCode = visibleInSourceCode;
  }

  public boolean isRangeSmaller(JavaAstNode other) {
    long other_start_line = other.getLocation_range_start_line();
    long other_end_line = other.getLocation_range_end_line();
    long other_start_column = other.getLocation_range_start_column();
    long other_end_column = other.getLocation_range_end_column();

    if (location_range_start_line == other_start_line) {
      if (location_range_end_line == other_end_line) {
        return location_range_end_column - location_range_start_column <
          other_end_column - other_start_column;
      }
      return location_range_end_line < other_end_line;
    } else if (location_range_end_line - location_range_start_line ==
      other_end_line - other_start_line) {
      return location_range_end_column - location_range_start_column <
        other_end_column - other_start_column;
    }
    return location_range_end_line - location_range_start_line <
      other_end_line - other_start_line;
  }
}
