package model;

import javax.persistence.*;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaTypedEntity\"")
@Inheritance(strategy = JOINED)
public abstract class JavaTypedEntity extends JavaEntity {
  @Column(name = "\"isFinal\"")
  private boolean isFinal;

  @Column(name = "\"isStatic\"")
  private boolean isStatic;

  @Column(name = "\"typeHash\"")
  private long typeHash;

  @Column(name = "\"qualifiedType\"")
  private String qualifiedType;


  // Getters and setters

  public boolean isFinal() {
    return isFinal;
  }

  public void setFinal(boolean aFinal) {
    isFinal = aFinal;
  }

  public boolean isStatic() {
    return isStatic;
  }

  public void setStatic(boolean aStatic) {
    isStatic = aStatic;
  }

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
