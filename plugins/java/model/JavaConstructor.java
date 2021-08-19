package model;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaConstructor\"")
public class JavaConstructor extends JavaEntity {
  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaConstructor_parameters\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaConVarParams = new HashSet<>();

  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaConstructor_locals\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaConVarLocals = new HashSet<>();

  public void addJavaConVarParam(JavaVariable javaVariable) {
    javaConVarParams.add(javaVariable);
    javaVariable.getJavaConstructorParams().add(this);
  }

  public void addJavaConVarLocal(JavaVariable javaVariable) {
    javaConVarLocals.add(javaVariable);
    javaVariable.getJavaConstructorLocals().add(this);
  }


  // Getters and setters

  public Set<JavaVariable> getJavaConVarParams() {
    return javaConVarParams;
  }

  public void setJavaConVarParams(Set<JavaVariable> javaConVarParams) {
    this.javaConVarParams = javaConVarParams;
  }

  public Set<JavaVariable> getJavaConVarLocals() {
    return javaConVarLocals;
  }

  public void setJavaConVarLocals(Set<JavaVariable> javaConVarLocals) {
    this.javaConVarLocals = javaConVarLocals;
  }
}
