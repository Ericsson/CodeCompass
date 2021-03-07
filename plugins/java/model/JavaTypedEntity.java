package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Inheritance;
import javax.persistence.Table;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaTypedEntity\"")
@Inheritance(strategy = JOINED)
public class JavaTypedEntity extends JavaEntity {
  @Column(name = "\"typeHash\"")
  private int typeHash;

  @Column(name = "\"qualifiedType\"")
  private String qualifiedType;


  // Getters and setters

  public int getTypeHash() {
    return typeHash;
  }

  public void setTypeHash(int typeHash) {
    this.typeHash = typeHash;
  }

  public String getQualifiedType() {
    return qualifiedType;
  }

  public void setQualifiedType(String qualifiedType) {
    this.qualifiedType = qualifiedType;
  }
}
