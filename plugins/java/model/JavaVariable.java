package model;

import javax.persistence.Entity;
import javax.persistence.ManyToMany;
import javax.persistence.Table;
import java.util.Set;

@Entity
@Table(name = "\"JavaVariable\"")
public class JavaVariable extends JavaTypedEntity {
  @ManyToMany(mappedBy = "javaMetVarParams")
  Set<JavaMethod> javaMethodParams;

  @ManyToMany(mappedBy = "javaMetVarLocals")
  Set<JavaMethod> javaMethodLocals;

  @ManyToMany(mappedBy = "javaConVarParams")
  Set<JavaConstructor> javaConstructorParams;

  @ManyToMany(mappedBy = "javaConVarLocals")
  Set<JavaConstructor> javaConstructorLocals;


  // Getters and setters

  public Set<JavaMethod> getJavaMethodParams() {
    return javaMethodParams;
  }

  public void setJavaMethodParams(Set<JavaMethod> javaMethodParams) {
    this.javaMethodParams = javaMethodParams;
  }

  public Set<JavaMethod> getJavaMethodLocals() {
    return javaMethodLocals;
  }

  public void setJavaMethodLocals(Set<JavaMethod> javaMethodLocals) {
    this.javaMethodLocals = javaMethodLocals;
  }

  public Set<JavaConstructor> getJavaConstructorParams() {
    return javaConstructorParams;
  }

  public void setJavaConstructorParams(
          Set<JavaConstructor> javaConstructorParams
  ) {
    this.javaConstructorParams = javaConstructorParams;
  }

  public Set<JavaConstructor> getJavaConstructorLocals() {
    return javaConstructorLocals;
  }

  public void setJavaConstructorLocals(
          Set<JavaConstructor> javaConstructorLocals
  ) {
    this.javaConstructorLocals = javaConstructorLocals;
  }
}