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

  @Column(name = "importer")
  private long importer;

  @Column(name = "imported")
  private long imported;

  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public long getImporter() {
    return importer;
  }

  public void setImporter(long importer) {
    this.importer = importer;
  }

  public long getImported() {
    return imported;
  }

  public void setImported(long imported) {
    this.imported = imported;
  }
}
