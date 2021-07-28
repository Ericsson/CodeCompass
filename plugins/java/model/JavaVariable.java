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
}
