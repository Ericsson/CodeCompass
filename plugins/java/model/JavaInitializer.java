package model;

import model.enums.InitializerKind;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaInitializer\"")
public class JavaInitializer extends JavaEntity {
  @Column(name = "\"kind\"")
  private InitializerKind kind;

  @Column(name = "\"typeHash\"")
  private long typeHash;

  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaInitializer_locals\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaInitVarLocals = new HashSet<>();

  public void addJavaInitVarLocal(JavaVariable javaVariable) {
    javaInitVarLocals.add(javaVariable);
    javaVariable.getJavaInitializerLocals().add(this);
  }

  // Getters and setters

  public InitializerKind getKind() {
    return kind;
  }

  public void setKind(InitializerKind kind) {
    this.kind = kind;
  }

  public long getTypeHash() {
    return typeHash;
  }

  public void setTypeHash(long typeHash) {
    this.typeHash = typeHash;
  }

  public Set<JavaVariable> getJavaInitVarLocals() {
    return javaInitVarLocals;
  }

  public void setJavaInitVarLocals(Set<JavaVariable> javaInitLocals) {
    this.javaInitVarLocals = javaInitLocals;
  }
}
