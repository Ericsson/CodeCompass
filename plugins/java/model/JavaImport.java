package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name = "\"JavaImport\"")
public class JavaImport {
  @Id
  @Column(name = "id")
  private int id;

  @Column(name = "imported")
  private int imported;

  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public int getImported() {
    return imported;
  }

  public void setImported(int imported) {
    this.imported = imported;
  }
}
