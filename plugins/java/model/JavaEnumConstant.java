package model;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaEnumConstant\"")
public class JavaEnumConstant extends JavaEntity {
  @Column(name = "value")
  private String value;

  @ManyToMany(mappedBy = "javaEnumConstants")
  Set<JavaEnum> javaEnums = new HashSet<>();


  // Getters and setters

  public String getValue() {
    return value;
  }

  public void setValue(String value) {
    this.value = value;
  }

  public Set<JavaEnum> getJavaEnums() {
    return javaEnums;
  }

  public void setJavaEnums(Set<JavaEnum> javaEnums) {
    this.javaEnums = javaEnums;
  }
}
