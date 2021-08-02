package model;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaEnum\"")
public class JavaEnum extends JavaEntity {
  @ManyToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinTable(
    name = "\"JavaEnum_enumConstants\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaEnumConstant> javaEnumConstants = new HashSet<>();

  public void addJavaEnumConstant(JavaEnumConstant enumConstant) {
    javaEnumConstants.add(enumConstant);
    enumConstant.getJavaEnums().add(this);
  }


  // Getters and setters

  public Set<JavaEnumConstant> getJavaEnumConstants() {
    return javaEnumConstants;
  }

  public void setJavaEnumConstants(Set<JavaEnumConstant> javaEnumConstants) {
    this.javaEnumConstants = javaEnumConstants;
  }
}
