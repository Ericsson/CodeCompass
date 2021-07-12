package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaDocComment\"")
public class JavaDocComment {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private int id;

  @Column(name = "\"contentHash\"")
  private long contentHash;

  @Column(name = "\"content\"")
  private int content;

  @Column(name = "\"entityHash\"")
  private long entityHash;

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

  public int getContent() {
    return content;
  }

  public void setContent(int content) {
    this.content = content;
  }

  public long getEntityHash() {
    return entityHash;
  }

  public void setMangledNameHash(long entityHash) {
    this.entityHash = entityHash;
  }

}
