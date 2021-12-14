package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaImport\"")
public class JavaImport {
  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  @Column(name = "id")
  private int id;

  @Column(name = "importer")
  private long importer;

  @Column(name = "imported")
  private long imported;

  @Column(name = "\"importedSymbol\"")
  private String importedSymbol;

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

  public String getImportedSymbol() {
    return importedSymbol;
  }

  public void setImportedSymbol(String importedSymbol) {
    this.importedSymbol = importedSymbol;
  }
}
