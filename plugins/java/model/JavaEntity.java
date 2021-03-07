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
  private int id;

  @Column(name = "\"typeId\"")
  private int typeId;

  @Column(name = "\"astNodeId\"")
  private int astNodeId;

  @Column(name = "\"mangledNameHash\"")
  private int mangledNameHash;

  @Column(name = "name")
  private String name;

  @Column(name = "\"qualifiedName\"")
  private String qualifiedName;


  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public int getTypeId() {
    return typeId;
  }

  public void setTypeId(int typeId) {
    this.typeId = typeId;
  }

  public int getAstNodeId() {
    return astNodeId;
  }

  public void setAstNodeId(int astNodeId) {
    this.astNodeId = astNodeId;
  }

  public int getMangledNameHash() {
    return mangledNameHash;
  }

  public void setMangledNameHash(int mangledNameHash) {
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
