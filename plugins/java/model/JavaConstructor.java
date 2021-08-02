package model;

import javax.persistence.*;
import java.util.Set;

@Entity
@Table(name = "\"JavaConstructor\"")
public class JavaConstructor extends JavaEntity {
    @ManyToMany
    @JoinTable(
      name = "\"JavaConstructor_parameters\"",
      joinColumns = @JoinColumn(name = "object_id"),
      inverseJoinColumns = @JoinColumn(name = "value")
    )
    Set<JavaVariable> javaConVarParams;

    @ManyToMany
    @JoinTable(
      name = "\"JavaConstructor_locals\"",
      joinColumns = @JoinColumn(name = "object_id"),
      inverseJoinColumns = @JoinColumn(name = "value")
    )
    Set<JavaVariable> javaConVarLocals;


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
