package model;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaMethod\"")
public class JavaMethod extends JavaTypedEntity {
  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaMethod_parameters\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaMetVarParams = new HashSet<>();

  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaMethod_locals\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaMetVarLocals = new HashSet<>();

  public void addJavaMetVarParam(JavaVariable javaVariable) {
    javaMetVarParams.add(javaVariable);
    javaVariable.getJavaMethodParams().add(this);
  }

  public void addJavaMetVarLocal(JavaVariable javaVariable) {
    javaMetVarLocals.add(javaVariable);
    javaVariable.getJavaMethodLocals().add(this);
  }


  // Getters and setters

  public Set<JavaVariable> getJavaMetVarParams() {
    return javaMetVarParams;
  }

  public void setJavaMetVarParams(Set<JavaVariable> javaMetVarParams) {
    this.javaMetVarParams = javaMetVarParams;
  }

  public Set<JavaVariable> getJavaMetVarLocals() {
    return javaMetVarLocals;
  }

  public void setJavaMetVarLocals(Set<JavaVariable> javaMetVarLocals) {
    this.javaMetVarLocals = javaMetVarLocals;
  }
}
