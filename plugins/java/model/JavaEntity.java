package model;

import javax.persistence.*;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaEntity\"")
@Inheritance(strategy = JOINED)
public class JavaEntity {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private long id;

  @Column(name = "\"typeId\"")
  private int typeId;

  @Column(name = "\"astNodeId\"")
  private long astNodeId;

  @Column(name = "\"mangledNameHash\"")
  private long mangledNameHash;

  @Column(name = "name")
  private String name;

  @Column(name = "\"qualifiedName\"")
  private String qualifiedName;


  // Getters and setters

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public int getTypeId() {
    return typeId;
  }

  public void setTypeId(int typeId) {
    this.typeId = typeId;
  }

  public long getAstNodeId() {
    return astNodeId;
  }

  public void setAstNodeId(long astNodeId) {
    this.astNodeId = astNodeId;
  }

  public long getMangledNameHash() {
    return mangledNameHash;
  }

  public void setMangledNameHash(long mangledNameHash) {
    this.mangledNameHash = mangledNameHash;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getQualifiedName() {
    return qualifiedName;
  }

  public void setQualifiedName(String qualifiedName) {
    this.qualifiedName = qualifiedName;
  }
}
