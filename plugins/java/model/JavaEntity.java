package model;

import javax.persistence.*;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaEntity\"")
@DiscriminatorColumn(
        name = "typeid",
        discriminatorType = DiscriminatorType.STRING
)
@Inheritance(strategy = JOINED)
public abstract class JavaEntity {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private long id;

  @Column(name = "\"astNodeId\"")
  private long astNodeId;

  @Column(name = "\"entityHash\"")
  private long entityHash;

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

  public long getAstNodeId() {
    return astNodeId;
  }

  public void setAstNodeId(long astNodeId) {
    this.astNodeId = astNodeId;
  }

  public long getEntityHash() {
    return entityHash;
  }

  public void setEntityHash(long mangledNameHash) {
    this.entityHash = mangledNameHash;
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
