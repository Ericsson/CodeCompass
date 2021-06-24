package model;

import javax.persistence.*;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaTypedEntity\"")
@Inheritance(strategy = JOINED)
public class JavaTypedEntity extends JavaEntity {
  @Column(name = "\"typeHash\"")
  private long typeHash;

  @Column(name = "\"qualifiedType\"")
  private String qualifiedType;


  // Getters and setters

  public long getTypeHash() {
    return typeHash;
  }

  public void setTypeHash(long typeHash) {
    this.typeHash = typeHash;
  }

  public String getQualifiedType() {
    return qualifiedType;
  }

  public void setQualifiedType(String qualifiedType) {
    this.qualifiedType = qualifiedType;
  }
}
