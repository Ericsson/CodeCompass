package model;

import javax.persistence.Entity;
import javax.persistence.Inheritance;
import javax.persistence.ManyToMany;
import javax.persistence.Table;
import java.util.Set;

import static javax.persistence.InheritanceType.JOINED;

@Entity
@Table(name = "\"JavaVariable\"")
public class JavaVariable extends JavaTypedEntity {
  @ManyToMany(mappedBy = "javaVariableParams")
  Set<JavaFunction> javaFunctionParams;

  @ManyToMany(mappedBy = "javaVariableLocals")
  Set<JavaFunction> javaFunctionLocals;
}
