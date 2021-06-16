package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name = "\"JavaDocComment\"")
public class JavaDocComment {
  @Id
  @Column(name = "id")
  private int id;

  @Column(name = "\"contentHash\"")
  private long contentHash;

  @Column(name = "\"contentHTML\"")
  private int contentHTML;

  @Column(name = "\"mangledNameHash\"")
  private long mangledNameHash;

// Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public long getContentHash() {
    return contentHash;
  }

  public void setContentHash(long contentHash) {
    this.contentHash = contentHash;
  }

  public int getContentHTML() {
    return contentHTML;
  }

  public void setContentHTML(int contentHTML) {
    this.contentHTML = contentHTML;
  }

  public long getMangledNameHash() {
    return mangledNameHash;
  }

  public void setMangledNameHash(long mangledNameHash) {
    this.mangledNameHash = mangledNameHash;
  }

}
