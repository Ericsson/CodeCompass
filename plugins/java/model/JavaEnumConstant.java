package model;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "\"JavaEnumConstant\"")
public class JavaEnumConstant extends JavaEntity {
  @Column(name = "value")
  private int value;

  @ManyToMany(mappedBy = "javaEnumConstants")
  Set<JavaEnum> javaEnums = new HashSet<>();


  // Getters and setters

  public int getValue() {
    return value;
  }

  public void setValue(int value) {
    this.value = value;
  }

  public Set<JavaEnum> getJavaEnums() {
    return javaEnums;
  }

  public void setJavaEnums(Set<JavaEnum> javaEnums) {
    this.javaEnums = javaEnums;
  }
}
