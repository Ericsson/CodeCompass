package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name = "\"JavaAstNode\"")
public class JavaAstNode {
  @Id
  @Column(name = "id")
  private int id;

  @Column(name = "\"astValue\"")
  private String astValue;

  @Column(name = "location_range_start_line")
  private int location_range_start_line;

  @Column(name = "location_range_start_column")
  private int location_range_start_column;

  @Column(name = "location_range_end_line")
  private int location_range_end_line;

  @Column(name = "location_range_end_column")
  private int location_range_end_column;

  @Column(name = "location_file")
  private int location_file;

  @Column(name = "\"mangledName\"")
  private String mangledName;

  @Column(name = "\"mangledNameHash\"")
  private int mangledNameHash;

  @Column(name = "\"symbolType\"")
  private int symbolType;

  @Column(name = "\"astType\"")
  private int astType;

  @Column(name = "\"visibleInSourceCode\"")
  private boolean visibleInSourceCode;


  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public String getAstValue() {
    return astValue;
  }

  public void setAstValue(String astValue) {
    this.astValue = astValue;
  }

  public int getLocation_range_start_line() {
    return location_range_start_line;
  }

  public void setLocation_range_start_line(int location_range_start_line) {
    this.location_range_start_line = location_range_start_line;
  }

  public int getLocation_range_start_column() {
    return location_range_start_column;
  }

  public void setLocation_range_start_column(int location_range_start_column) {
    this.location_range_start_column = location_range_start_column;
  }

  public int getLocation_range_end_line() {
    return location_range_end_line;
  }

  public void setLocation_range_end_line(int location_range_end_line) {
    this.location_range_end_line = location_range_end_line;
  }

  public int getLocation_range_end_column() {
    return location_range_end_column;
  }

  public void setLocation_range_end_column(int location_range_end_column) {
    this.location_range_end_column = location_range_end_column;
  }

  public int getLocation_file() {
    return location_file;
  }

  public void setLocation_file(int location_file) {
    this.location_file = location_file;
  }

  public String getMangledName() {
    return mangledName;
  }

  public void setMangledName(String mangledName) {
    this.mangledName = mangledName;
  }

  public int getMangledNameHash() {
    return mangledNameHash;
  }

  public void setMangledNameHash(int mangledNameHash) {
    this.mangledNameHash = mangledNameHash;
  }

  public int getSymbolType() {
    return symbolType;
  }

  public void setSymbolType(int symbolType) {
    this.symbolType = symbolType;
  }

  public int getAstType() {
    return astType;
  }

  public void setAstType(int astType) {
    this.astType = astType;
  }

  public boolean isVisibleInSourceCode() {
    return visibleInSourceCode;
  }

  public void setVisibleInSourceCode(boolean visibleInSourceCode) {
    this.visibleInSourceCode = visibleInSourceCode;
  }
}
