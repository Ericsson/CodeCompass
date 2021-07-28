package model;

import javax.persistence.*;
import java.util.Set;

@Entity
@Table(name = "\"JavaMethod\"")
public class JavaMethod extends JavaTypedEntity {
  @ManyToMany
  @JoinTable(
    name = "\"JavaMethod_parameters\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaMetVarParams;

  @ManyToMany
  @JoinTable(
    name = "\"JavaMethod_locals\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaVariable> javaMetVarLocals;
}
