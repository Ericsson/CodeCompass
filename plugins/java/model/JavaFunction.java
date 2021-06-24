package model;

import javax.persistence.*;
import java.util.Set;

@Entity
@Table(name = "\"JavaFunction\"")
public class JavaFunction extends JavaTypedEntity {
  @ManyToMany
  @JoinTable(
    name = "\"JavaFunction_parameters\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaVariableParams;

  @ManyToMany
  @JoinTable(
    name = "\"JavaFunction_locals\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaVariableLocals;
}
