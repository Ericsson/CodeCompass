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
}
